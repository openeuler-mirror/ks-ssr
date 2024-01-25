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

#pragma once

#include <QDBusContext>
#include <QList>
#include <QReadWriteLock>
#include "src/daemon/account/manager.h"
#include "src/daemon/log/configuration.h"
#include "src/daemon/log/message.h"

class QFileSystemWatcher;
class QWaitCondition;
class QProcess;
class QFile;

class WriteWorker;

#define SSR_LOG(logType, logMsg, result, dbusId)                                 \
    {                                                                            \
        auto role = KS::Account::Manager::m_accountManager->getRole(dbusId);     \
        auto name = KS::Account::Manager::m_accountManager->getUserName(dbusId); \
        auto timePoint = QDateTime::currentDateTime();                           \
        KS::Log::Log log{name, role, timePoint, logType, result, logMsg};          \
        KS::Log::Manager::writeLog(log);                                         \
    }

#define SSR_LOG_SUCCESS(logType, logMsg, dbusId)                                 \
    {                                                                            \
        auto role = KS::Account::Manager::m_accountManager->getRole(dbusId);     \
        auto name = KS::Account::Manager::m_accountManager->getUserName(dbusId); \
        auto timePoint = QDateTime::currentDateTime();                           \
        KS::Log::Log log{name, role, timePoint, logType, true, logMsg};          \
        KS::Log::Manager::writeLog(log);                                         \
    }

#define SSR_LOG_ERROR(logType, logMsg, dbusId)                                   \
    {                                                                              \
        auto role = KS::Account::Manager::m_accountManager->getRole(dbusId);     \
        auto name = KS::Account::Manager::m_accountManager->getUserName(dbusId); \
        auto timePoint = QDateTime::currentDateTime();                             \
        KS::Log::Log log{name, role, timePoint, logType, false, logMsg};           \
        KS::Log::Manager::writeLog(log);                                           \
    }

// Qt 自身的文件读写就有一个大小为 16384 大小的缓冲区，所以在此类中不再做缓冲
namespace KS
{
namespace Log
{
class RealTimeAlert;
struct Log;
class Manager : public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    enum LogType
    {
        ERROR = -1,
        DEVICE = (1 << 0),
        TOOL_BOX = (1 << 1),
        BASELINE_REINFORCEMENT = (1 << 2),
        TRUSTED_PROTECTION = (1 << 3),
        FILES_PROTECTION = (1 << 4),
        PRIVATE_BOX = (1 << 5),
        ACCOUNT = (1 << 6)
    };
    Q_ENUM(LogType)

private:
    Manager();
    ~Manager();

public:
    static void globalInit();
    static void globalDeinit();
    static void writeLog(const Log& log);
    uint GetLogNum(const int role, const time_t begin_time_stamp, const time_t end_time_stamp, const int type, const uint result, const QString& searchText);
    QStringList GetLog(const int role, const time_t time_stamp, const time_t end_time_stamp, const int type, const uint result, const QString& searchText, const uint per_page, const uint page) const;
    // bool SetLogRotateConfig(const QString& config);

private:
    void backUpLog(const QStringList& targetLogList);
    void getAllLog();
    QStringList getLogFileList(bool isReverse) const;
    void logFileRotate();

Q_SIGNALS:  // SIGNALS
    void NewLogWritten(uint log_num);
    void needLogRotate();

public:
    static Manager* m_logManager;

private:
    // 当前日志文件的行数
    uint m_fileLine;
    QString m_path;
    QFile* m_file;
    QProcess* m_backUpLogProcess;
    const Configurations m_configurations;
    // 日志数据结构选用 List 容器。
    QList<Log> m_logList;
    // 第一个未写入元素的下标
    uint m_firstNeedWrite;
    QWaitCondition* m_waitCondition;
    // 临界资源日志队列的锁
    QReadWriteLock m_listMutex;
    // 临界资源日志文件的锁
    QMutex m_fileMutex;
    QThread* m_thread;

    friend class WriteWorker;
};

struct Log
{
    QString name;
    Account::Manager::AccountRole role;
    QDateTime timeStamp;
    Manager::LogType type;
    bool result;
    QString logMsg;
};
};  // namespace Log
};  // namespace KS
