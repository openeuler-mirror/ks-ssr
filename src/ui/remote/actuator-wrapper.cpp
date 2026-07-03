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
#include "actuator-wrapper.h"
#include <qt5-log-i.h>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

namespace KS
{
namespace RemotePage
{
ActuatorWrapper::ActuatorWrapper(QObject *parent)
    : QObject(parent),
      m_process(new QProcess(this)),
      m_debugFile(nullptr)
{
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &ActuatorWrapper::onProcessReadReady);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ActuatorWrapper::onProcessFinished);
}

ActuatorWrapper::~ActuatorWrapper()
{
    m_process->terminate();
    m_process->waitForFinished();
    delete m_debugFile;
}

int ActuatorWrapper::checkMachineLoginConfig(const QString &path)
{
    if (!QFileInfo::exists(path))
    {
        return -1;
    }

    QFileInfo fileInfo(path);
    if (!fileInfo.isReadable())
    {
        return -1;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return -1;
    }

    int count = 0;
    while (!file.atEnd())
    {
        QString line = file.readLine();
        line.replace(QRegExp("\\s"), "");
        auto splitRes = line.split(',');

        if (splitRes.size() != 2)
            continue;

        ++count;
    }

    return count;
}

bool ActuatorWrapper::isRunning()
{
    return m_running;
}

#define ACTUATOR_SCRIPT_DIR "/usr/share/ks-ssr/distribution-actuator/"
bool ActuatorWrapper::start(const QString &path, bool reinforce, bool vulnerabilityRepair)
{
    if (isRunning())
    {
        KLOG_WARNING() << "actuator is already running";
        return false;
    }

    auto logPath = QString("/tmp/ssr-distribution-actuator-%1.log")
                       .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-hh:mm:ss"));

    m_debugFile = new QFile(logPath);
    if (m_debugFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        appendDebugLog(QString("start actuator for %1").arg(path));
    }
    else
    {
        delete m_debugFile;
        m_debugFile = nullptr;
    }

    auto scriptCmd = QString("%1/%2 excute %3 %4 %5")
                         .arg(ACTUATOR_SCRIPT_DIR,
                              "distribution-actuator.sh",
                              path,
                              reinforce ? "reinforce" : "",
                              vulnerabilityRepair ? "repair" : "");
    KLOG_INFO() << "script cmd:" << scriptCmd;

    m_process->setWorkingDirectory(ACTUATOR_SCRIPT_DIR);
    m_process->start("/bin/bash", {"-c", scriptCmd});
    if (!m_process->waitForStarted())
    {
        appendDebugLog(QString("start script failed,%1").arg(m_process->errorString()));
        delete m_debugFile;
        return false;
    }

    m_running = true;
    return true;
}

bool ActuatorWrapper::stop()
{
    if (!m_running)
        return false;

    m_process->terminate();
    m_process->waitForFinished();
    return true;
}

void ActuatorWrapper::appendDebugLog(const QString &msg)
{
    QTextStream stream;
    if (m_debugFile)
    {
        stream.setDevice(m_debugFile);
    }
    stream << msg << '\n';
}

bool ActuatorWrapper::isInternalCommand(const QString &line)
{
    static const QString internalComandPrefix = "kylinsec_ssr_actuator_feedback:";
    if (!line.startsWith(internalComandPrefix))
    {
        return false;
    }
    return true;
}

void ActuatorWrapper::parseInternalCommand(const QString &line)
{
    if (!isInternalCommand(line))
    {
        return;
    }

    auto splitRes = line.split(" ");
    if (splitRes.size() < 3)
    {
        return;
    }

    splitRes.removeFirst();
    auto cmd = splitRes.takeFirst();
    auto info = splitRes.join(" ");

    if (cmd == "total-job")
    {
        auto totalCount = info.toInt();
        m_totalCount = totalCount;
        emit started(m_totalCount);
    }
    else if (cmd == "job-done")
    {
        auto infoSplitRes = info.split(",");
        if (infoSplitRes.size() != 2)
        {
            KLOG_WARNING() << "parse internal error:" << line;
            return;
        }
        auto address = infoSplitRes.at(0);
        bool success = infoSplitRes.at(1) == "0";
        success ? m_successedEntries << address : m_failedEntries << address;
        emit entryCompleted(m_successedEntries.count() + m_failedEntries.count());
    }
    else if (cmd == "debug")
    {
        appendDebugLog(info);
        echoLog(info);
    }
}

void ActuatorWrapper::onProcessReadReady()
{
    auto standardOutput = m_process->readAllStandardOutput();
    QTextStream stream(standardOutput);
    while (!stream.atEnd())
    {
        parseInternalCommand(stream.readLine());
    }
}

void ActuatorWrapper::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    KLOG_INFO() << "process finished:" << exitCode << exitStatus;

    int successedCount = m_successedEntries.count();
    int failedCount = m_totalCount - successedCount;

    auto summary = QString("finished,total <%1> succeed <%2> failed <%3>")
                       .arg(m_totalCount)
                       .arg(successedCount)
                       .arg(failedCount);
    appendDebugLog(summary);

    if (failedCount == 0)
    {
        emit finished(true,
                      QString(tr("Execution completed, %1 successful executions").arg(successedCount)),
                      m_debugFile ? m_debugFile->fileName() : "");
    }
    else
    {
        emit finished(false,
                      QString(tr("Execution completed, %1 successful executions, %2 failed executions").arg(successedCount).arg(failedCount)),
                      m_debugFile ? m_debugFile->fileName() : "");
    }

    if (m_debugFile)
    {
        delete m_debugFile;
        m_debugFile = nullptr;
    }

    m_totalCount = 0;
    m_successedEntries.clear();
    m_failedEntries.clear();

    m_running = false;
}
}  // namespace RemotePage
}  // namespace KS