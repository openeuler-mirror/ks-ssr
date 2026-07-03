/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */

#pragma once
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QVector>

namespace KS
{
namespace RemotePage
{
class ActuatorWrapper : public QObject
{
    Q_OBJECT
public:
    explicit ActuatorWrapper(QObject* parent = nullptr);
    ~ActuatorWrapper();

    /**
     * 检查机器登录配置
     * @param path 配置文件路径
     * @return -1: 配置不正常 0: 无有效配置 其他: 其中配置有效的数量
     */
    static int checkMachineLoginConfig(const QString& path);

    bool isRunning();
    bool start(const QString& path, bool reinforce, bool vulnerabilityRepair);
    bool stop();

signals:
    void started(quint64 total);
    void entryCompleted(quint64 current);
    void echoLog(const QString& log);
    void finished(bool isSuccessed, const QString& msg, const QString& logPath);

private:
    void appendDebugLog(const QString& msg);
    bool isInternalCommand(const QString& line);
    void parseInternalCommand(const QString& line);
    void onJobDone(const QString& line);

private slots:
    void onProcessReadReady();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess* m_process = nullptr;
    bool m_running = false;
    QFile* m_debugFile = nullptr;
    int m_totalCount = 0;
    QVector<QString> m_failedEntries;
    QVector<QString> m_successedEntries;
};
}  // namespace RemotePage
}  // namespace KS