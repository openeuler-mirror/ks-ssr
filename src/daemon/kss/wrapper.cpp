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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */

#include "wrapper.h"
#include <qt5-log-i.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QFile>
#include "config.h"
#include "include/ssr-i.h"
#include "include/ssr-marcos.h"

namespace KS
{
namespace KSS
{
// 命令是否存在
#define KSS_CMD_PATH SSR_INSTALL_BINDIR "/kss"

// ini文件
#define KSS_INI_PATH SSR_INSTALL_DATADIR "/ssr.ini"
#define KSS_INI_KEY "ssr/initialized"
// 线程初始化 等待时长（30分钟）
#define KSS_INIT_THREAD_TIMEOUT 30 * 60 * 1000

#define KSS_JSON_KEY_DATA_PATH SSR_KSS_JK_DATA_PATH

// 暂时使用 -p 传入user pin，后续做需要做可信卡时再做修改
#define KSS_INIT_CMD "kss card deploy -p"
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
#define KSS_DIGEST_SCAN_CMD "kss digest scan"
#define KSS_DIGEST_INFO_GET_EXECUTE_CMD "kss digest info -e"
#define KSS_DIGEST_INFO_GET_KERNEL_CMD "kss digest info -m"

#define KSS_ADD_FILE_CMD "kss file add"
#define KSS_REMOVE_FILE_CMD "kss file del"
#define KSS_GET_FILES_CMD "kss file info"

// 开启防卸载
#define KSS_OPEN_PROHIBIT_UNLOADING_CMD "kss kmod add --path"
// 关闭防卸载
#define KSS_CLOSE_PROHIBIT_UNLOADING_CMD "kss kmod del --path"

Wrapper::Wrapper(QObject *parent) : QObject(parent), m_kssInitThread(nullptr)
{
    m_process = new QProcess(parent);
    m_ini = new QSettings(KSS_INI_PATH, QSettings::IniFormat, this);
}

QSharedPointer<Wrapper> Wrapper::m_instance = nullptr;
QSharedPointer<Wrapper> Wrapper::getDefault()
{
    if (!m_instance)
    {
        m_instance = QSharedPointer<Wrapper>::create();
        m_instance->init();
    }
    return m_instance;
}

QString Wrapper::addTrustedFile(const QString &filePath)
{
    RETURN_VAL_IF_TRUE(getInitialized() == 0, QString())
    auto cmd = QString("%1 -u '%2'").arg(KSS_DIGEST_SCAN_CMD, filePath);
    execute(cmd);

    return m_processOutput;
}

QString Wrapper::addTrustedFiles(const QString &json)
{
    RETURN_VAL_IF_TRUE(getInitialized() == 0, QString())
    auto cmd = QString("%1 -u -j '%2'").arg(KSS_DIGEST_SCAN_CMD, json);
    execute(cmd);

    return m_processOutput;
}

void Wrapper::removeTrustedFile(const QString &filePath)
{
    RETURN_IF_TRUE(getInitialized() == 0)

    // 需要判断内核模块是否开启防卸载
    QJsonParseError jsonError;
    auto trustedFilesJson = getTrustedFiles(SSRKSSTrustedFileType::SSR_KSS_TRUSTED_FILE_TYPE_KERNEL).toUtf8();
    auto jsonDoc = QJsonDocument::fromJson(trustedFilesJson, &jsonError);
    
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
        return;
    }

    auto jsonModules = jsonDoc.object().value(SSR_KSS_JK_DATA).toArray();
    for (const auto &module : jsonModules)
    {
        auto jsonMod = module.toObject();
        if (filePath != jsonMod.value(SSR_KSS_JK_DATA_PATH).toString())
        {
            continue;
        }
        if (jsonMod.value(SSR_KSS_JK_DATA_GUARD).toInt() == 0)
        {
            continue;
        }

        // path正确且防卸载开启则需先关闭防卸载
        prohibitUnloading(false, filePath);
    }
    // 移除
    auto cmd = QString("%1 -f -r '%2'").arg(KSS_DIGEST_SCAN_CMD, filePath);
    execute(cmd);
}

void Wrapper::removeTrustedFiles(const QString &json)
{
    RETURN_IF_TRUE(getInitialized() == 0)

    // 需要判断内核模块是否开启防卸载
    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(getTrustedFiles(SSRKSSTrustedFileType::SSR_KSS_TRUSTED_FILE_TYPE_KERNEL).toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser information failed: " << jsonError.errorString();
        return;
    }

    auto jsonModules = jsonDoc.object().value(SSR_KSS_JK_DATA).toArray();
    for (const auto &module : jsonModules)
    {
        auto jsonMod = module.toObject();
        if (!json.contains(jsonMod.value(SSR_KSS_JK_DATA_PATH).toString()))
        {
            continue;
        }
        if (jsonMod.value(SSR_KSS_JK_DATA_GUARD).toInt() == 0)
        {
            continue;
        }

        // path正确且防卸载开启则需先关闭防卸载
        prohibitUnloading(false, jsonMod.value(SSR_KSS_JK_DATA_PATH).toString());
    }
    // 移除
    auto cmd = QString("%1 -r -j '%2'").arg(KSS_DIGEST_SCAN_CMD, json);
    execute(cmd);
}

QString Wrapper::getTrustedFiles(SSRKSSTrustedFileType type)
{
    RETURN_VAL_IF_TRUE(getInitialized() == 0, QString())
    RETURN_VAL_IF_TRUE(type == SSRKSSTrustedFileType::SSR_KSS_TRUSTED_FILE_TYPE_NONE, QString())

    execute(type == SSRKSSTrustedFileType::SSR_KSS_TRUSTED_FILE_TYPE_EXECUTE ? KSS_DIGEST_INFO_GET_EXECUTE_CMD : KSS_DIGEST_INFO_GET_KERNEL_CMD);
    return m_processOutput;
}

void Wrapper::prohibitUnloading(bool prohibited, const QString &filePath)
{
    auto cmd = QString("%1 %2").arg(prohibited ? KSS_OPEN_PROHIBIT_UNLOADING_CMD : KSS_CLOSE_PROHIBIT_UNLOADING_CMD, filePath);
    execute(cmd);
}

QString Wrapper::setStorageMode(SSRKSSTrustedStorageType type, const QString &userPin)
{
    RETURN_VAL_IF_TRUE(getInitialized() == 0, QString("Failed to set storage mode"))
    RETURN_VAL_IF_TRUE(type == SSRKSSTrustedStorageType::SSR_KSS_TRUSTED_STORAGE_TYPE_NONE, QString("Failed to set storage mode"))

    auto cmd = QString("%1 %2").arg(type == SSRKSSTrustedStorageType::SSR_KSS_TRUSTED_STORAGE_TYPE_SOFT ? KSS_SECURE_TCM_SOFT_CMD : KSS_SECURE_TCM_HARD_CMD, userPin);
    execute(cmd);

    return m_errorOutput;
}

SSRKSSTrustedStorageType Wrapper::getCurrentStorageMode()
{
    execute(KSS_CARD_INFO_CMD);

    if (m_processOutput.trimmed() == "Software Backend")
    {
        return SSRKSSTrustedStorageType::SSR_KSS_TRUSTED_STORAGE_TYPE_SOFT;
    }
    else
    {
        return SSRKSSTrustedStorageType::SSR_KSS_TRUSTED_STORAGE_TYPE_HARD;
    }
}

void Wrapper::setTrustedStatus(bool status)
{
    RETURN_IF_TRUE(getInitialized() == 0)
    auto cmd = QString("%1 %2").arg(KSS_SECURE_SET_STATUS_CMD, status ? "-s" : "-l");
    execute(cmd);
}

QString Wrapper::getTrustedStatus()
{
    execute(KSS_SECURE_GET_STATUS_CMD);
    return m_processOutput;
}

void Wrapper::addFile(const QString &fileName, const QString &filePath, const QString &insertTime)
{
    RETURN_IF_TRUE(getInitialized() == 0)
    auto cmd = QString("%1 -n '%2' -p '%3' -t '%4'").arg(KSS_ADD_FILE_CMD, fileName, filePath, insertTime);
    execute(cmd);
}

void Wrapper::addFiles(const QString &json)
{
    RETURN_IF_TRUE(getInitialized() == 0)
    auto cmd = QString("%1 -j '%2'").arg(KSS_ADD_FILE_CMD, json);
    execute(cmd);
}

void Wrapper::removeFile(const QString &filePath)
{
    RETURN_IF_TRUE(getInitialized() == 0)
    auto cmd = QString("%1 -p '%2'").arg(KSS_REMOVE_FILE_CMD, filePath);
    execute(cmd);
}

void Wrapper::removeFiles(const QString &json)
{
    RETURN_IF_TRUE(getInitialized() == 0)
    auto cmd = QString("%1 -j '%2'").arg(KSS_REMOVE_FILE_CMD, json);
    execute(cmd);
}

QString Wrapper::getFiles()
{
    RETURN_VAL_IF_TRUE(getInitialized() == 0, QString())
    execute(KSS_GET_FILES_CMD);
    return m_processOutput;
}

int Wrapper::getInitialized()
{
    return m_ini->value(KSS_INI_KEY).toInt();
}

void Wrapper::processExited(int exitCode, QProcess::ExitStatus exitStatus)
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

void Wrapper::execute(const QString &cmd)
{
    KLOG_DEBUG() << "Start executing the command. cmd = " << cmd;
    m_process->start("bash", QStringList() << "-c" << cmd);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processExited(int, QProcess::ExitStatus)));
    // 一分钟超时
    m_process->waitForFinished(60 * 1000);
}

void Wrapper::initTrustedResults()
{
    KLOG_INFO() << "Kss data initialisation completed.";

    m_ini->setValue(KSS_INI_KEY, 1);
    emit initFinished();
}

void Wrapper::init()
{
    RETURN_IF_TRUE(m_ini->value(KSS_INI_KEY).toInt() != 0);
    RETURN_IF_TRUE(!QFile::exists(KSS_CMD_PATH));

    KLOG_INFO() << "Start kss initialisation.";
    execute(QString("%1 %2").arg(KSS_INIT_CMD, KSS_DEFAULT_USER_PIN));

    m_kssInitThread = QThread::create([this]
                                      {
                                          auto process = new QProcess(this);
                                          auto cmd = QString("%1 %2").arg(KSS_INIT_DATA_CMD, "/boot/vmlinuz-`uname -r`");
                                          KLOG_DEBUG() << "Start executing the command. cmd = " << cmd;
                                          process->start("bash", QStringList() << "-c" << cmd);
                                          process->waitForFinished(KSS_INIT_THREAD_TIMEOUT);
                                      });

    connect(m_kssInitThread, &QThread::finished, this, &Wrapper::initTrustedResults);
    connect(m_kssInitThread, &QThread::finished, m_kssInitThread, &QThread::deleteLater);

    m_kssInitThread->start();
}
}  // namespace KSS
}  // namespace KS
