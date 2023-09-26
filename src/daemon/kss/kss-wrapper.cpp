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

#include "kss-wrapper.h"
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
#define KSS_INI_PATH KSC_INSTALL_DATADIR "/ksc.ini"
#define KSS_INI_KEY "ksc/initialized"
// 线程初始化 等待时长（30分钟）
#define KSS_INIT_THREAD_TIMEOUT 30 * 60 * 1000

#define KSS_JSON_KEY_DATA_PATH KSC_KSS_JK_DATA_PATH

// 暂时使用 -p 传入user pin，后续做需要做可信卡时再做修改
#define KSS_INIT_CMD "kss card deploy -p 123123"
#define KSS_INIT_DATA_CMD "kss secure setup"

// 软硬件模式切换
#define KSS_SECURE_TCM_SOFT_CMD "kss secure tcm -s -p"
#define KSS_SECURE_TCM_HARD_CMD "kss secure tcm -p"
// 获取当前软硬模式信息
#define KSS_CARD_INFO_CMD "kss card info"

// 可信开关 -s开启，-l关闭
#define KSS_SECURE_SET_STATUS_CMD "kss secure set-status"
// 获取可信状态
#define KSS_SECURE_GET_STATUS_CMD "kss secure get-status"

// 可信数据操作
#define KSS_DIGEST_SCAN_ADD_FILE_CMD "kss digest scan -u"
#define KSS_DIGEST_SCAN_REMOVE_FILE_CMD "kss digest scan -f -r"
#define KSS_DIGEST_INFO_GET_EXECUTE_CMD "kss digest info -e"
#define KSS_DIGEST_INFO_GET_KERNEL_CMD "kss digest info -m"

#define KSS_ADD_FILE_CMD "kss file add"
#define KSS_REMOVE_FILE_CMD "kss file del"
#define KSS_GET_FILES_CMD "kss file info"

// 开启防卸载
#define KSS_OPEN_PROHIBIT_UNLOADING_CMD "kss kmod add --path"
// 关闭防卸载
#define KSS_CLOSE_PROHIBIT_UNLOADING_CMD "kss kmod del --path"

KSSWrapper::KSSWrapper(QObject *parent) : QObject(parent), m_kssInitThread(nullptr)
{
    m_process = new QProcess(parent);
    m_ini = new QSettings(KSS_INI_PATH, QSettings::IniFormat, this);
}

QSharedPointer<KSSWrapper> KSSWrapper::m_instance = nullptr;
QSharedPointer<KSSWrapper> KSSWrapper::getDefault()
{
    if (!m_instance)
    {
        m_instance = QSharedPointer<KSSWrapper>::create();
        m_instance->init();
    }
    return m_instance;
}

QString KSSWrapper::addTrustedFile(const QString &filePath)
{
    RETURN_VAL_IF_TRUE(getInitialized() == 0, QString())
    auto cmd = QString("%1 %2").arg(KSS_DIGEST_SCAN_ADD_FILE_CMD, filePath);
    execute(cmd);

    return m_processOutput;
}

void KSSWrapper::removeTrustedFile(const QString &filePath)
{
    RETURN_IF_TRUE(getInitialized() == 0)
    auto cmd = QString("%1 %2").arg(KSS_DIGEST_SCAN_REMOVE_FILE_CMD, filePath);
    execute(cmd);
}

QString KSSWrapper::getTrustedFiles(KSCKSSTrustedFileType type)
{
    RETURN_VAL_IF_TRUE(getInitialized() == 0, QString())
    RETURN_VAL_IF_TRUE(type == KSCKSSTrustedFileType::KSC_KSS_TRUSTED_FILE_TYPE_NONE, QString())

    execute(type == KSCKSSTrustedFileType::KSC_KSS_TRUSTED_FILE_TYPE_EXECUTE ? KSS_DIGEST_INFO_GET_EXECUTE_CMD : KSS_DIGEST_INFO_GET_KERNEL_CMD);
    return m_processOutput;
}

void KSSWrapper::prohibitUnloading(bool prohibited, const QString &filePath)
{
    auto cmd = QString("%1 %2").arg(prohibited ? KSS_OPEN_PROHIBIT_UNLOADING_CMD : KSS_CLOSE_PROHIBIT_UNLOADING_CMD, filePath);
    execute(cmd);
}

QString KSSWrapper::setStorageMode(KSCKSSTrustedStorageType type, const QString &userPin)
{
    RETURN_VAL_IF_TRUE(getInitialized() == 0, QString())
    RETURN_VAL_IF_TRUE(type == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_NONE, QString())

    auto cmd = QString("%1 %2").arg(type == KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT ? KSS_SECURE_TCM_SOFT_CMD : KSS_SECURE_TCM_HARD_CMD, userPin);
    execute(cmd);

    return m_errorOutput;
}

KSCKSSTrustedStorageType KSSWrapper::getCurrentStorageMode()
{
    execute(KSS_CARD_INFO_CMD);

    if (m_processOutput.trimmed() == "Software Backend")
    {
        return KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT;
    }
    else
    {
        return KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_HARD;
    }
}

void KSSWrapper::setTrustedStatus(bool status)
{
    RETURN_IF_TRUE(getInitialized() == 0)
    auto cmd = QString("%1 %2").arg(KSS_SECURE_SET_STATUS_CMD, status ? "-s" : "-l");
    execute(cmd);
}

QString KSSWrapper::getTrustedStatus()
{
    execute(KSS_SECURE_GET_STATUS_CMD);
    return m_processOutput;
}

void KSSWrapper::addFile(const QString &fileName, const QString &filePath, const QString &insertTime)
{
    RETURN_IF_TRUE(getInitialized() == 0)
    auto cmd = QString("%1 -n %2 -p %3 -t '%4'").arg(KSS_ADD_FILE_CMD, fileName, filePath, insertTime);
    execute(cmd);
}

void KSSWrapper::removeFile(const QString &filePath)
{
    RETURN_IF_TRUE(getInitialized() == 0)
    auto cmd = QString("%1 -p %2").arg(KSS_REMOVE_FILE_CMD, filePath);
    execute(cmd);
}

QString KSSWrapper::getFiles()
{
    RETURN_VAL_IF_TRUE(getInitialized() == 0, QString())
    execute(KSS_GET_FILES_CMD);
    return m_processOutput;
}

int KSSWrapper::getInitialized()
{
    return m_ini->value(KSS_INI_KEY).toInt();
}

void KSSWrapper::processExited(int exitCode, QProcess::ExitStatus exitStatus)
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

void KSSWrapper::execute(const QString &cmd)
{
    KLOG_DEBUG() << "Start executing the command. cmd = " << cmd;
    m_process->start("bash", QStringList() << "-c" << cmd);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processExited(int, QProcess::ExitStatus)));
    // 一分钟超时
    m_process->waitForFinished(60 * 1000);
}

void KSSWrapper::initTrustedResults()
{
    KLOG_INFO() << "Kss data initialisation completed.";

    m_ini->setValue(KSS_INI_KEY, 1);
    emit initFinished();
}

void KSSWrapper::init()
{
    RETURN_IF_TRUE(m_ini->value(KSS_INI_KEY).toInt() != 0)

    KLOG_INFO() << "Start kss initialisation.";
    execute(KSS_INIT_CMD);

    m_kssInitThread = QThread::create([this]
                                      {
                                          auto process = new QProcess(this);
                                          auto cmd = QString("%1 %2").arg(KSS_INIT_DATA_CMD, "/boot/vmlinuz-`uname -r`");
                                          KLOG_DEBUG() << "Start executing the command. cmd = " << cmd;
                                          process->start("bash", QStringList() << "-c" << cmd);
                                          process->waitForFinished(KSS_INIT_THREAD_TIMEOUT);
                                      });

    connect(m_kssInitThread, &QThread::finished, this, &KSSWrapper::initTrustedResults);
    connect(m_kssInitThread, &QThread::finished, m_kssInitThread, &QThread::deleteLater);

    m_kssInitThread->start();
}
}  // namespace KS
