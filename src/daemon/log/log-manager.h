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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#pragma once

#include <daemon-accounts-i.h>
#include <daemon-log-i.h>
#include <QDBusContext>
#include <QList>
#include <QMutex>
#include <QReadWriteLock>
#include "configuration.h"
#include "message.h"

class QFileSystemWatcher;
class QWaitCondition;
class QProcess;
class QTimer;
class QFile;

class WriteWorker;

// Qt 自身的文件读写就有一个大小为 16384 大小的缓冲区，所以在此类中不再做缓冲
namespace KS
{
namespace Accounts
{
class Manager;
}

namespace Log
{
class RealTimeAlert;
struct LogRecord;

class Manager : public QObject, public IDaemonLog, protected QDBusContext
{
    Q_OBJECT
public:
    Manager(IDaemonAccounts* accountManager);
    virtual ~Manager();

public:
    virtual void writeLog(LogType logType, const QString& logMsg, bool result, const QString& dbusID);
    virtual void writeLog(const QString& name, int role, QDateTime timestamp, LogType logType, bool result, const QString& logMsg);

    void writeLog(const LogRecord& log);
    uint GetLogNum(const int role,
                   const time_t begin_time_stamp,
                   const time_t end_time_stamp,
                   const int type,
                   const uint result,
                   const QString& searchText);
    QStringList GetLog(const int role,
                       const time_t begin_time_stamp,
                       const time_t end_time_stamp,
                       const int type,
                       const uint result,
                       const QString& searchText,
                       const uint per_page,
                       const uint page);

    static QString logTypeEnum2Str(LogType logType);
    static LogType logTypeStr2Enum(const QString& logTypeStr);

private:
    void backUpLog(const QStringList& targetLogList);
    void getAllLog();
    QStringList getLogFileList(bool isReverse) const;
    void logFileRotateInTimer();
    void logFileRotate();

Q_SIGNALS:  // SIGNALS
    void NewLogWritten(uint log_num);
    void needLogRotate();

private:
    IDaemonAccounts* m_accountManager;
    // 当前日志文件的行数
    uint m_fileLine;
    QString m_path;
    QFile* m_file;
    QProcess* m_backUpLogProcess;
    QProcess* m_cleanUpLogProcess;
    const Configurations m_configurations;
    // 日志数据结构选用 List 容器。
    QList<LogRecord> m_logList;
    // 第一个未写入元素的下标
    uint m_firstNeedWrite;
    QWaitCondition* m_waitCondition;
    // 临界资源日志队列的锁
    QReadWriteLock m_listMutex;
    // 临界资源日志文件的锁
    QMutex m_fileMutex;
    QThread* m_thread;
    QTimer* m_bakUpTimer;

    friend class WriteWorker;
};

struct LogRecord
{
    QString name;
    int role;
    QDateTime timeStamp;
    LogType type;
    bool result;
    QString logMsg;
};
};  // namespace Log
};  // namespace KS
