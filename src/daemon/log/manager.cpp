/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#include "src/daemon/log/manager.h"
#include "src/daemon/log/write-worker.h"
#include <src/daemon/common/dbus-helper.h>
#include <src/daemon/log_adaptor.h>
#include <ssr-marcos.h>
#include <QDir>
#include <QHostAddress>
#include <QStringBuilder>
#include "config.h"
#include <src/daemon/log/message.h>

#define AUDITD_CONF "/etc/audit/auditd.conf"
#define SSR_LOG_DBUS_OBJECT_PATH "/com/kylinsec/SSR/Log"
#define LOGFILEDIR SSR_INSTALL_DATADIR "/log/"
#define LOGFILENAME "ks-ssr.log"
#define ABSOLUTELOGFILEPATH LOGFILEDIR LOGFILENAME

namespace KS
{
namespace Log
{

Manager* Manager::m_logManager = nullptr;

Manager::Manager()
    : m_path(QDir::cleanPath(ABSOLUTELOGFILEPATH)),
      m_file(new QFile(m_path, this)),
      m_watcher(new QFileSystemWatcher(this)),
      m_backUpLogProcess(new QProcess(this)),
      m_configurations(),
      m_messageQueue(new QQueue<QString>()),
      m_waitCondition(new QWaitCondition()),
      m_thread(new WriteWorker(m_messageQueue, m_file, m_waitCondition, &m_queueMutex, &m_fileMutex, this))
{
    // 初始化日志文件
    QDir logFilePath;
    if (!logFilePath.exists(LOGFILEDIR))
    {
        KLOG_INFO() << "Log file dir doesn't exist, make it.";
        if (!logFilePath.mkdir(LOGFILEDIR))
        {
            KLOG_ERROR() << "Cannot make log dir!";
        }
    }
    KLOG_INFO() << "Message Path " << m_path;
    if (!m_file->open(QIODevice::ReadWrite | QIODevice::Append))
    {
        KLOG_ERROR() << "Failed to open log file !";
    }

    // 启动将日志写入磁盘的工作线程
    m_thread->start();

    // 设置备份日志文件
    QString program = "sshpass";
    QStringList arg;
    arg << "-p" << m_configurations.m_passwd << "scp" << LOGFILENAME "-*"
        << QString("%1@%2").arg(m_configurations.m_account, m_configurations.m_ip.toString()) << LOGFILEDIR;
    m_backUpLogProcess->setProgram(program);
    m_backUpLogProcess->setArguments(arg);
    connect(m_backUpLogProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [this](int, QProcess::ExitStatus) {
        QString logMsg{};
        QTextStream logMsgStream{&logMsg};
        uint rc = m_backUpLogProcess->exitCode();
        if (rc != 0)
        {
            KLOG_WARNING() << "Failed to back up log!";
        }
        logMsgStream << "Back up return value: " << rc
                     << ", output: " << m_backUpLogProcess->readAllStandardOutput();
        logMsgStream.flush();
        writeLog(Message{Message::LogType::LOG, logMsg}.serialize());
    });

    // 初始化 DBus 接口
    new LogAdaptor(this);
    QDBusConnection dbusConnection = QDBusConnection::systemBus();
    if (!dbusConnection.registerObject(SSR_LOG_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "Register Log DBus object error:" << dbusConnection.lastError().message();
    }
    if (!m_watcher->addPath(m_path))
    {
        KLOG_ERROR() << "Failed to make log file watcher, logrotate is disable";
    }
    connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &Manager::logFileChanged);
}

Manager::~Manager()
{
    delete m_file;
    delete m_backUpLogProcess;
    delete m_messageQueue;
}

void Log::Manager::globalInit()
{
    if (Manager::m_logManager == nullptr)
    {
        m_logManager = new Manager();
    }
    KLOG_WARNING() << "LogManager has been init";
}

void Manager::globalDeinit()
{
    delete m_logManager;
}

QStringList Manager::GetLog(const uint per_page, const uint page)
{
    auto callerPid = DBusHelper::getCallerUniqueName(m_logManager);
    RETURN_VAL_IF_TRUE(callerPid == -1, QStringList());

    Manager::writeLog(Message{Message::LogType::LOG, "get logs"}.serialize());
    QMutexLocker locker(&Manager::m_logManager->m_fileMutex);

    KLOG_DEBUG() << "Get logs, per page limit is " << per_page << "page index is " << page;
    if ((per_page == 0 || per_page > 500) || page == 0)
    {
        KLOG_ERROR() << "per page limit and page index must greater than 0";
        return QStringList();
    }
    // 读之前将所有缓冲区的内容写入到日志文件中。
    if (!m_logManager->m_file->flush())
    {
        KLOG_ERROR() << "Failed to flush log, Error: "
                     << m_logManager->m_file->errorString();
        return QStringList();
    }

    // 日志文件名称的List，排序： ks-ssr.log， ks-ssr.log.1， ks-ssr.log.2...
    auto logFilesList = m_logManager->getLogFileList(false);
    // 将要返回的日志 list
    QStringList logList{};
    // 需要忽略的日志数量
    auto offset = (page - 1) * per_page;
    while (!logFilesList.isEmpty())
    {
        auto logFileName = logFilesList.takeFirst();
        QFile logFile(LOGFILEDIR + logFileName);
        if (!logFile.open(QIODevice::OpenModeFlag::ReadOnly))
        {
            KLOG_ERROR() << "Failed to Open log file " << logFileName
                         << ", error str: " << logFile.errorString();
            return QStringList();
        }
#warning "以文件行数为轮转标准，避免遍历文件来获取文件行数"
        while (!logFile.atEnd())
        {
            if (static_cast<uint>(logList.count()) == (per_page))
            {
                std::reverse(logList.begin(), logList.end());
                return logList;
            }
            // 抛弃当前行
            if (offset != 0)
            {
                logFile.readLine();
                offset--;
                continue;
            }
            logList << logFile.readLine();
        }
    }
    std::reverse(logList.begin(), logList.end());
    return logList;
}

void Log::Manager::backUpLog()
{
    if (m_configurations.m_ip.isNull())
    {
        writeLog(Message{Message::LogType::LOG, "No target ip, skip backup log"}.serialize());
        KLOG_INFO() << "No target ip, skip backup log";
        return;
    }
    if (m_backUpLogProcess->state() != QProcess::ProcessState::NotRunning)
    {
        KLOG_INFO() << "Last backup process is running, skip.";
        return;
    }
    m_backUpLogProcess->start();
}

inline QStringList Manager::getLogFileList(bool isReverse)
{
    int sortMode = QDir::Name;
    if (isReverse)
    {
        sortMode |= QDir::Reversed;
    }
    QDir logDir(LOGFILEDIR);
    return logDir.entryList(QStringList() << LOGFILENAME "*",
                            QDir::NoDotAndDotDot | QDir::Files,
                            static_cast<QDir::SortFlag>(sortMode));
}

void Manager::writeLog(const QString& log)
{
    // 先将消息加入消息队列，如果日志可写则唤醒工作线程
    QMutexLocker locker(&m_logManager->m_queueMutex);
    m_logManager->m_messageQueue->enqueue(log);
    if (m_logManager->m_file == nullptr || !m_logManager->m_file->isWritable())
    {
        KLOG_WARNING() << "Cannot write log!";
        return;
    }
    m_logManager->m_waitCondition->notify_one();
}

void Manager::logFileChanged(const QString&)
{
    // 大小单位转换成 M
    if (m_file->size() / (1024 * 1024) < m_configurations.m_maxLogFile)
    {
        return;
    }
    // 如果此时 watch 的文件数量为空，则代表此次文件改变事件是被 logFileChanged 本身触发，需忽略。
    // QFileSystemWatcher 的 remove 操作本身是线程安全的
    if (!m_watcher->removePath(m_path))
    {
        KLOG_INFO() << "Other handler is processing, ignore";
        return;
    }
    QMutexLocker locker(&m_fileMutex);
    delete m_file;
    // 按编号顺序排序，第一个是编号最大的
    auto logLists = getLogFileList(true);
    KLOG_DEBUG() << "Log lists: " << logLists.join(", ");

    // 当前已有的日志文件数量小于配置文件中的设置，所以需要新建日志文件
    constexpr auto sepIndex = sizeof(LOGFILENAME) - 1;

    // logDiffNum 用于保存当前文件和配置文件中日志数量的差值
    // 当 logDiffNum 是负数时，证明当前日志文件数量小于日志中的数量，所以需要新建一个日志文件
    // 当 logDiffNum 大于等于 0 时，证明需要移除部分文件，需要移除的数量为 logDiffNum + 1, 加一是为了为新日志腾出空间
    int logDiffNum = logLists.count() - m_configurations.m_numLogs;
    bool isNeedBackUpLog = false;
    KLOG_DEBUG() << "Will remove files " << logDiffNum;
    while (logDiffNum >= 0)
    {
        isNeedBackUpLog = true;
        const auto logFile = logLists.takeFirst();
        if (!QFile::rename(logFile,
                           LOGFILENAME "-" + QDateTime::currentDateTime().toString() + "." + QString(logDiffNum)))
        {
            KLOG_WARNING() << "Failed to rename log file: " << logFile;
        }
        logDiffNum--;
    }

    locker.unlock();
    if (isNeedBackUpLog)
    {
        if (static_cast<uint>(logLists.count()) != m_configurations.m_numLogs - 1)
        {
            KLOG_ERROR() << "Internal error: Current log nums shoule be config's numLogs - 1 !"
                         << "log nums = " << logLists.count() << ", config's numLogs = " << m_configurations.m_numLogs;
            return;
        }
        backUpLog();
    }
    locker.relock();
    // 此时当前存在的日志数量应该为 m_configurations->m_numLogs -1, 将所有日志文件的编号加一然后再新建日志文件即可
    // 将当前所有日志文件编号加一
    for (const auto& logName : logLists)
    {
        if (!QFile::rename(logName, LOGFILENAME "." + logName.mid(sepIndex).toInt() + 1))
        {
            KLOG_ERROR() << "Failed to mv " << logName << " to " << LOGFILENAME + logName.mid(sepIndex).toInt() + 1;
        }
    }

    // 新建日志
    m_file = new QFile(m_path);
    if (!m_file->open(QIODevice::ReadWrite | QIODevice::Append))
    {
        KLOG_ERROR() << "failed to open log file !";
    }
    m_watcher->addPath(m_path);
}
};  // namespace Log
};  // namespace KS