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
#include <QDir>
#include <QHostAddress>
#include <QStringBuilder>
#include <QTimer>
#include "config.h"
#include "include/ssr-error-i.h"
#include "include/ssr-i.h"
#include "include/ssr-marcos.h"
#include "lib/base/error.h"
#include "manager.h"
#include "src/daemon/account/manager.h"
#include "src/daemon/common/dbus-helper.h"
#include "src/daemon/log/message.h"
#include "src/daemon/log/write-worker.h"
#include "src/daemon/log_adaptor.h"
#include "ssr-marcos.h"

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
    : m_fileLine(0),
      m_path(QDir::cleanPath(ABSOLUTELOGFILEPATH)),
      m_file(new QFile(m_path, this)),
      m_backUpLogProcess(new QProcess(this)),
      m_cleanUpLogProcess(new QProcess(this)),
      m_configurations(),
      m_waitCondition(new QWaitCondition()),
      m_thread(new WriteWorker(this)),
      m_bakUpTimer(new QTimer(this))
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
    if (!m_file->open(QIODevice::ReadWrite))
    {
        KLOG_ERROR() << "Failed to open log file !";
    }

    // 内存需要管理的数量大小应该和当前本地所有日志数量一致
    // m_logList 的初始化，它需要将本地所有的日志读入内存
    // 注意 ks-ssr.log 和其他的 ks-ssr.log.* 是分开读取的， 因为还需要获取 ks-ssr.log 当前的日志条数
    getAllLog();
    while (!m_file->atEnd())
    {
        m_logList.append(Message::deserialize(m_file->readLine()));
        m_fileLine++;
    }
    KLOG_DEBUG() << "Log nums: " << m_logList.size();
    m_firstNeedWrite = m_logList.size();

    // 启动将日志写入磁盘的工作线程
    m_thread->start();

    // 设置备份日志文件
    connect(m_backUpLogProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [this](int, QProcess::ExitStatus)
            {
                uint rc = m_backUpLogProcess->exitCode();
                QString errMsg = m_backUpLogProcess->readAllStandardOutput() +
                                 m_backUpLogProcess->readAllStandardError();
                if (rc != 0)
                {
                    KLOG_WARNING() << "error code: " << rc
                                   << ", Failed to back up overflow log! error message: " << errMsg;
                    // SSR_LOG(Account::Manager::AccountRole::unknown_account, LogType::LOG, "Failed to back up overflow log!", false);
                }
            });
    connect(m_cleanUpLogProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [this](int, QProcess::ExitStatus)
            {
                uint rc = m_cleanUpLogProcess->exitCode();
                QString errMsg = m_cleanUpLogProcess->readAllStandardOutput() +
                                 m_cleanUpLogProcess->readAllStandardError();
                if (rc != 0)
                {
                    KLOG_WARNING() << "error code: " << rc
                                   << ", Failed to clean up back up log! error message: " << errMsg;
                    // SSR_LOG(Account::Manager::AccountRole::unknown_account, LogType::LOG, "Failed to back up overflow log!", false);
                }
            });

    // 初始化 DBus 接口
    new LogAdaptor(this);
    QDBusConnection dbusConnection = QDBusConnection::systemBus();
    if (!dbusConnection.registerObject(SSR_LOG_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "Register Log DBus object error:" << dbusConnection.lastError().message();
    }
    QObject::connect(this, &Manager::needLogRotate, this, &Manager::logFileRotate, Qt::QueuedConnection);
    connect(m_bakUpTimer, &QTimer::timeout, this, &KS::Log::Manager::logFileRotateInTimer);
    m_bakUpTimer->setInterval(m_configurations.m_bakUpInterval * 1000);
    m_bakUpTimer->start();
}

Manager::~Manager()
{
    delete m_file;
    delete m_backUpLogProcess;
}

void Manager::globalInit()
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

uint Manager::GetLogNum(const int role, const time_t begin_time_stamp, const time_t end_time_stamp, const int type, const uint result, const QString& searchText)
{
    QReadLocker locker(&m_listMutex);
    uint logSize = 0;
    auto reverseIt = m_logList.rbegin();
    auto reverseEnd = m_logList.rend();
    while (reverseIt != reverseEnd)
    {
        // 为了可读性，将判断条件取反了
        if ((static_cast<int>(reverseIt->role) & role) == 0)
        {
            reverseIt++;
            continue;
        }
        if (reverseIt->timeStamp.toSecsSinceEpoch() < begin_time_stamp ||
            reverseIt->timeStamp.toSecsSinceEpoch() >= end_time_stamp)
        {
            reverseIt++;
            continue;
        }
        if ((static_cast<int>(reverseIt->type) & type) == 0)
        {
            reverseIt++;
            continue;
        }
        // 当前端传入 LOG_RESULT_ALL 时代表需要所有结果的日志， 所以不针对日志的结果筛选
        if (result != LogResult::LOG_RESULT_ALL)
        {
            if (!(static_cast<uint>(reverseIt->result) == result))
            {
                reverseIt++;
                continue;
            }
        }
        if (!reverseIt->logMsg.contains(searchText, Qt::CaseSensitivity::CaseInsensitive))
        {
            reverseIt++;
            continue;
        }
        logSize++;
        reverseIt++;
    }
    return logSize;
}

QStringList Manager::GetLog(const int role, const time_t begin_time_stamp, const time_t end_time_stamp, const int type, const uint result, const QString& searchText, const uint per_page, const uint page) const
{
    auto callerUnique = DBusHelper::getCallerUniqueName(m_logManager);
    auto _role = Account::Manager::m_accountManager->getRole(callerUnique);
    if (_role == KS::Account::Manager::AccountRole::unknown_account)
    {
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QStringList(), SSRErrorCode::ERROR_ACCOUNT_UNKNOWN_ACCOUNT, this->message());
    }

    KLOG_DEBUG() << "Get logs, per page limit is " << per_page << "page index is " << page;
    if ((per_page == 0 || per_page > 500) || page == 0)
    {
        KLOG_ERROR() << "per page limit must less than 100 and page index must greater than 0.";
        DBUS_ERROR_REPLY_AND_RETURN_VAL(QStringList(), SSRErrorCode::ERROR_LOG_GET_LOG_PAGE_ERROR, this->message());
    }
    QReadLocker locker(&m_logManager->m_listMutex);

    // 日志文件名称的List，排序： ks-ssr.log， ks-ssr.log.1， ks-ssr.log.2...
    // 临时变量，用于存储 Log 类型的数据，在锁释放后再序列化
    QList<Log> tmpLogList{};
    // 需要忽略的日志数量
    auto totalOffset = (page - 1) * per_page;

    // 最新的日志在最前面
    auto reverseIt = m_logList.rbegin();
    auto reverseEnd = m_logList.rend();
    while (reverseIt != reverseEnd && static_cast<uint>(tmpLogList.size()) < per_page)
    {
        // 为了可读性，将判断条件取反了
        if ((static_cast<int>(reverseIt->role) & role) == 0)
        {
            reverseIt++;
            continue;
        }
        if (reverseIt->timeStamp.toSecsSinceEpoch() < begin_time_stamp ||
            reverseIt->timeStamp.toSecsSinceEpoch() >= end_time_stamp)
        {
            reverseIt++;
            continue;
        }
        if ((static_cast<int>(reverseIt->type) & type) == 0)
        {
            reverseIt++;
            continue;
        }
        // 当前端传入 LOG_RESULT_ALL 时代表需要所有结果的日志， 所以不针对日志的结果筛选
        if (result != LogResult::LOG_RESULT_ALL)
        {
            if (!(static_cast<uint>(reverseIt->result) == result))
            {
                reverseIt++;
                continue;
            }
        }
        if (!reverseIt->logMsg.contains(searchText, Qt::CaseSensitivity::CaseInsensitive))
        {
            reverseIt++;
            continue;
        }
        if (totalOffset != 0)
        {
            totalOffset--;
            reverseIt++;
            continue;
        }
        tmpLogList.append(*reverseIt);
        reverseIt++;
    }
    locker.unlock();
    QStringList retLogList{};
    for (const auto& log : tmpLogList)
    {
        retLogList << Message::serialize(log);
    }
    // SSR_LOG(_role, LogType::LOG, "Get Log");
    return retLogList;
}

void Manager::backUpLog(const QStringList& targetLogList)
{
    if (m_configurations.m_ip.isNull())
    {
        KLOG_INFO() << "No target ip, skip back up log";
        return;
    }
    if (m_backUpLogProcess->state() != QProcess::ProcessState::NotRunning)
    {
        KLOG_INFO() << "Last backup process is running, skip.";
        return;
    }
    // 入参是不能修改的，所以构造一个副本。
    QStringList _targetLogList{};
    for (auto& log : targetLogList)
    {
        _targetLogList.append(LOGFILEDIR + log);
    }
    const QString bakUpProgram = "sshpass";

    QStringList bakUpArg;
    bakUpArg << "-p" << m_configurations.m_passwd << "rsync" << _targetLogList
             << QString("%1@%2:%3").arg(m_configurations.m_account, m_configurations.m_ip.toString(), m_configurations.m_remotePath);
    m_backUpLogProcess->setProgram(bakUpProgram);
    m_backUpLogProcess->setArguments(bakUpArg);

    QStringList removeLogList{};
    for (auto& log : targetLogList)
    {
        removeLogList.append(m_configurations.m_remotePath + '/' + log);
    }

    QStringList cleanUpArg;
    cleanUpArg << "-p" << m_configurations.m_passwd << "ssh"
               << QString("%1@%2").arg(m_configurations.m_account, m_configurations.m_ip.toString())
               << QString("bash -c 'echo \"rm %1\" | sudo at now + 6 months'").arg(removeLogList.join(' '));
    m_cleanUpLogProcess->setProgram(bakUpProgram);
    m_cleanUpLogProcess->setArguments(cleanUpArg);

    m_backUpLogProcess->start();
    m_backUpLogProcess->waitForFinished();
    m_cleanUpLogProcess->start();
    m_cleanUpLogProcess->waitForFinished();
}

void Manager::getAllLog()
{
    auto logList = getLogFileList(true);
    // 去掉 ks-ssr.log
    logList.removeLast();
    for (const auto& log : logList)
    {
        // 跳过溢出的日志文件。
        if (log.startsWith(LOGFILENAME "-"))
        {
            continue;
        }
        QFile logFile(LOGFILEDIR + log);
        if (!logFile.open(QIODevice::OpenModeFlag::ReadOnly))
        {
            KLOG_ERROR() << "Failed to open log file: " << logFile.fileName();
            continue;
        }
        while (!logFile.atEnd())
        {
            m_logList.append(Message::deserialize(logFile.readLine()));
        }
    }
}

inline QStringList Manager::getLogFileList(bool isReverse) const
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

void Manager::writeLog(const Log& log)
{
    // 先将消息加入消息队列，如果日志可写则唤醒工作线程
    QWriteLocker locker(&m_logManager->m_listMutex);
    m_logManager->m_logList.append(log);
    emit m_logManager->NewLogWritten(m_logManager->m_logList.size());
    // 当日志的数据结构大于本地能容纳的日志数量时，去掉最老的那一条日志，也就是第一条日志。
    // 旧日志的删除或备份由日志轮转功能保证。
    if (static_cast<uint>(m_logManager->m_logList.size()) >
        (m_logManager->m_configurations.m_maxLogFileLine * m_logManager->m_configurations.m_numLogs))
    {
        m_logManager->m_logList.removeFirst();
        m_logManager->m_firstNeedWrite--;
    }
    locker.unlock();
    if (m_logManager->m_file == nullptr || !m_logManager->m_file->isWritable())
    {
        KLOG_WARNING() << "Cannot write log!";
        return;
    }
    m_logManager->m_waitCondition->notify_one();
}

void Manager::logFileRotateInTimer()
{
    QMutexLocker locker(&m_fileMutex);
    m_fileLine = 0;
    delete m_file;
    m_file = nullptr;
    auto logList = getLogFileList(true);
    auto index = logList.size();
    for (const auto& log : logList)
    {
        if (!QFile::rename(LOGFILEDIR + log,
                           LOGFILEDIR LOGFILENAME "-" + QDateTime::currentDateTime().toString(Qt::ISODate) + "." + QString::number(index)))
        {
            KLOG_WARNING() << "Failed to rename log file: " << log;
        }
        index--;
    }
    logList = getLogFileList(true);
    backUpLog(logList);
    QDir logDir(LOGFILEDIR);
    for (const auto& log : logList)
    {
        logDir.remove(log);
    }
    m_file = new QFile(m_path);
    if (!m_file->open(QIODevice::ReadWrite | QIODevice::Append))
    {
        KLOG_ERROR() << "failed to open log file !";
    }
}

void Manager::logFileRotate()
{
    QMutexLocker locker(&m_fileMutex);
    if (m_fileLine < m_configurations.m_maxLogFileLine)
    {
        KLOG_INFO() << "The log file has been rotated, ignored";
        return;
    }
    m_fileLine = 0;
    delete m_file;
    // 按编号顺序排序，第一个是编号最大的
    auto logLists = getLogFileList(true);
    // 移除不需要关注的日志
    for (auto& logName : logLists)
    {
        if (!logName.startsWith(LOGFILENAME "-"))
        {
            continue;
        }
        logLists.removeOne(logName);
    }
    KLOG_DEBUG() << "Log lists: " << logLists.join(", ");

    // 当前已有的日志文件数量小于配置文件中的设置，所以需要新建日志文件
    constexpr auto sepIndex = sizeof(LOGFILENAME);

    // logDiffNum 用于保存当前文件和配置文件中日志数量的差值
    // 当 logDiffNum 是负数时，证明当前日志文件数量小于日志中的数量，所以需要新建一个日志文件
    // 当 logDiffNum 大于等于 0 时，证明需要移除部分文件，需要移除的数量为 logDiffNum + 1, 加一是为了为新日志腾出空间
    int logDiffNum = logLists.count() - m_configurations.m_numLogs;
    bool isNeedBackUpLog = false;
    KLOG_DEBUG() << "Will remove files num: " << (logDiffNum < 0 ? 0 : logDiffNum + 1);
    // 将大于轮转数量的日志重命名为可备份的名称
    // 如当前可轮转的数量为 5, 则 ks-ssr.log.6 会被重命名为 ks-ssr-${ISO_FORMAT_CURRENT_TIME}.log.6
    // 然后将 ks-ssr.log-${ISO_FORMAT_CURRENT_TIME}.${NUM} 迁移至日志备份服务器
    while (logDiffNum >= 0)
    {
        isNeedBackUpLog = true;
        const auto logFile = logLists.takeFirst();
        if (!QFile::rename(LOGFILEDIR + logFile,
                           LOGFILEDIR LOGFILENAME "-" + QDateTime::currentDateTime().toString(Qt::ISODate) + "." + QString::number(logDiffNum)))
        {
            KLOG_WARNING() << "Failed to rename log file: " << logFile;
        }
        logDiffNum--;
    }
    // 此时当前存在的日志数量应该为 m_configurations->m_numLogs -1, 将所有日志文件的编号加一然后再新建日志文件即可
    // 将当前所有日志文件编号加一
    for (const auto& logName : logLists)
    {
        auto newLogName = LOGFILENAME "." + QString::number(logName.mid(sepIndex).toInt() + 1);
        if (!QFile::rename(LOGFILEDIR + logName,
                           LOGFILEDIR + newLogName))
        {
            KLOG_ERROR() << "Failed to mv " << logName << " to " << newLogName;
        }
    }

    // 新建日志
    m_file = new QFile(m_path);
    if (!m_file->open(QIODevice::ReadWrite | QIODevice::Append))
    {
        KLOG_ERROR() << "failed to open log file !";
    }
    auto overFlowLogList = getLogFileList(true);
    for (const auto& log : overFlowLogList)
    {
        if (log.startsWith(LOGFILENAME "-"))
        {
            continue;
        }
        overFlowLogList.removeOne(log);
    }
    if (isNeedBackUpLog)
    {
        if (static_cast<uint>(logLists.count()) != m_configurations.m_numLogs - 1)
        {
            KLOG_ERROR() << "Internal error: Current log nums shoule be config's numLogs - 1 !"
                         << "log nums = " << logLists.count() << ", config's numLogs = " << m_configurations.m_numLogs;
            return;
        }
        backUpLog(overFlowLogList);
    }
    // 无论是否备份到远程，都需要删除超出的文件
    QDir logDir(LOGFILEDIR);
    for (const auto& log : overFlowLogList)
    {
        logDir.remove(log);
    }

    // 通知执行线程完成未完成的工作
    m_waitCondition->notify_one();
}
};  // namespace Log
};  // namespace KS
