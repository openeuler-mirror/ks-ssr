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
#include <QMutex>
#include "src/daemon/log/configuration.h"

class QFileSystemWatcher;
class QProcess;
class QWaitCondition;
class QFile;

// Qt 自身的文件读写就有一个大小为 16384 大小的缓冲区，所以在此类中不再做缓冲
namespace KS
{
#define SSR_LOG()

// 使用示例
// Manager::writeLog(Message{Message::LogType::${TYPE}, "something"}.serialize())
namespace Log
{
class Manager : public QObject, protected QDBusContext
{
    Q_OBJECT
private:
    Manager();
    ~Manager();

public:
    static void globalInit();
    static void globalDeinit();
    static void writeLog(const QString& log);
    static QStringList GetLog(const uint per_page, const uint page);
    bool SetLogRotateConfig(const QString& config);

private:
    void backUpLog();
    QStringList getLogFileList(bool isReverse);

private slots:
    void logFileChanged(const QString& path);

public:
    static Manager* m_logManager;

private:
    QString m_path;
    QFile* m_file;
    QFileSystemWatcher* m_watcher;
    QProcess* m_backUpLogProcess;
    const Configurations m_configurations;
    QQueue<QString>* m_messageQueue;
    QWaitCondition* m_waitCondition;
    QThread* m_thread;
    QMutex m_queueMutex;
    QMutex m_fileMutex;
};

};  // namespace Log
};  // namespace KS