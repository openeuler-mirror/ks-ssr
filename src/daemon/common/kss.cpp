/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */

#include "kss.h"
#include <qt5-log-i.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include "config.h"
#include "include/ksc-i.h"
#include "include/ksc-marcos.h"

namespace KS
{
// ini文件
#define KSC_INI_PATH KSC_INSTALL_DATADIR "/ksc.ini"
#define KSC_INI_KEY "ksc/initialized"
// 线程初始化 等待时长（30分钟）
#define KSS_INIT_THREAD_TIMEOUT 30 * 60 * 1000

#define KSS_JSON_KEY_DATA_PATH KSC_JK_DATA_PATH

// 暂时使用 -p 传入user pin，后续做需要做可信卡时再做修改
#define KSS_INIT_CMD "kss card deploy -p 123123"
#define KSS_INIT_DATA_CMD "kss secure setup"

#define KSS_DIGEST_SCAN_ADD_FILE_CMD "kss digest scan -u"
#define KSS_DIGEST_SCAN_REMOVE_FILE_CMD "kss digest scan -r"
#define KSS_DIGEST_INFO_GET_EXECUTE_CMD "kss digest info -e"
#define KSS_DIGEST_INFO_GET_KERNEL_CMD "kss digest info -m"

#define KSS_ADD_FILE_CMD "kss file add"
#define KSS_REMOVE_FILE_CMD "kss file del"
#define KSS_GET_FILES_CMD "kss file info"

// 开启防卸载
#define KSS_OPEN_PROHIBIT_UNLOADING_CMD "kss kmod add --path"
// 关闭防卸载
#define KSS_CLOSE_PROHIBIT_UNLOADING_CMD "kss kmod del --path"

KSS::KSS(QObject *parent) : QObject(parent), m_kssInitThread(nullptr)
{
    m_process = new QProcess(parent);
    m_ini = new QSettings(KSC_INI_PATH, QSettings::IniFormat, this);
}

void KSS::addTrustedFile(const QString &filePath)
{
    RETURN_IF_TRUE(m_ini->value(KSC_INI_KEY).toInt() == 0)
    auto cmd = QString("%1 %2").arg(KSS_DIGEST_SCAN_ADD_FILE_CMD, filePath);
    execute(cmd);
}

void KSS::removeTrustedFile(const QString &filePath)
{
    RETURN_IF_TRUE(m_ini->value(KSC_INI_KEY).toInt() == 0)
    auto cmd = QString("%1 %2").arg(KSS_DIGEST_SCAN_REMOVE_FILE_CMD, filePath);
    execute(cmd);
}

QString KSS::getModuleFiles()
{
    RETURN_VAL_IF_TRUE(m_ini->value(KSC_INI_KEY).toInt() == 0, QString())
    execute(KSS_DIGEST_INFO_GET_KERNEL_CMD);
    return m_processOutput;
}

void KSS::prohibitUnloading(bool prohibited, const QString &filePath)
{
    auto cmd = QString("%1 %2").arg(prohibited ? KSS_OPEN_PROHIBIT_UNLOADING_CMD : KSS_CLOSE_PROHIBIT_UNLOADING_CMD, filePath);
    execute(cmd);
}

QString KSS::getExecuteFiles()
{
    RETURN_VAL_IF_TRUE(m_ini->value(KSC_INI_KEY).toInt() == 0, QString())
    execute(KSS_DIGEST_INFO_GET_EXECUTE_CMD);
    return m_processOutput;
}

void KSS::addFile(const QString &fileName, const QString &filePath, const QString &insertTime)
{
    RETURN_IF_TRUE(m_ini->value(KSC_INI_KEY).toInt() == 0)
    auto cmd = QString("%1 -n %2 -p %3 -t '%4'").arg(KSS_ADD_FILE_CMD, fileName, filePath, insertTime);
    execute(cmd);
}

void KSS::removeFile(const QString &filePath)
{
    RETURN_IF_TRUE(m_ini->value(KSC_INI_KEY).toInt() == 0)
    auto cmd = QString("%1 -p %2").arg(KSS_REMOVE_FILE_CMD, filePath);
    execute(cmd);
}

QString KSS::getFiles()
{
    RETURN_VAL_IF_TRUE(m_ini->value(KSC_INI_KEY).toInt() == 0, QString())
    execute(KSS_GET_FILES_CMD);
    return m_processOutput;
}

void KSS::processExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    KLOG_DEBUG() << "Command execution completed. exitcode = " << exitCode << "exitStatus = " << exitStatus;
    m_process->disconnect();

    auto standardOutput = m_process->readAllStandardOutput();

//    KLOG_DEBUG() << "Execute the command to successfully output: " << standardOutput;
    m_processOutput = standardOutput;

    auto errordOutput = m_process->readAllStandardError();
    if (!errordOutput.isEmpty())
    {
        KLOG_ERROR() << "Execution command error output: " << errordOutput;
    }
    m_errorOutput = errordOutput;
}

void KSS::execute(const QString &cmd)
{
    KLOG_DEBUG() << "Start executing the command. cmd = " << cmd;
    m_process->start("bash", QStringList() << "-c" << cmd);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processExited(int, QProcess::ExitStatus)));
    m_process->waitForFinished();
}

void KSS::initTrustedResults()
{
    KLOG_INFO() << "Kss data initialisation completed.";

    m_ini->setValue(KSC_INI_KEY, 1);
    emit initFinished();
}

void KSS::initTrusted()
{
    RETURN_IF_TRUE(m_ini->value(KSC_INI_KEY).toInt() != 0)

    KLOG_INFO() << "Start kss initialisation.";
    execute(KSS_INIT_CMD);

    m_kssInitThread = QThread::create([this]
                                      {
                                          auto process = new QProcess(this);
                                          auto cmd = QString("%1 %2").arg(KSS_INIT_DATA_CMD, "/boot/vmlinuz-`uname -r` -b");
                                          KLOG_DEBUG() << "Start executing the command. cmd = " << cmd;
                                          process->start("bash", QStringList() << "-c" << cmd);
                                          process->waitForFinished(KSS_INIT_THREAD_TIMEOUT);
                                      });

    connect(m_kssInitThread, &QThread::finished, this, &KSS::initTrustedResults);
    connect(m_kssInitThread, &QThread::finished, m_kssInitThread, &QThread::deleteLater);

    m_kssInitThread->start();
}
}  // namespace KS
