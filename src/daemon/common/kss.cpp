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
#include "include/sc-i.h"
#include "include/sc-marcos.h"

namespace KS
{
#define KSS_INIT_CMD "kss card deploy"
#define KSS_INIT_DATA_CMD "kss secure setup"

#define DIGEST_SCAN_ADD_FILE_CMD "kss digest scan -u"
#define DIGEST_SCAN_REMOVE_FILE_CMD "kss digest scan -r"
#define DIGEST_SCAN_GET_EXECUTE_CMD "kss digest info -e"
#define DIGEST_SCAN_GET_KERNEL_CMD "kss digest info -m"

#define ADD_FILE_CMD "kss file add"
#define REMOVE_FILE_CMD "kss file del"
#define GET_FILES_CMD "kss file info"

KSS::KSS(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(parent);
    m_ini = new QSettings(KSS_INI_PATH, QSettings::IniFormat, this);

    this->initData();
}

void KSS::addTrustedFile(const QString &filePath)
{
    auto cmd = QString("%1 %2").arg(DIGEST_SCAN_ADD_FILE_CMD, filePath);
    this->execute(cmd);
}

void KSS::removeTrustedFile(const QString &filePath)
{
    auto cmd = QString("%1 %2").arg(DIGEST_SCAN_REMOVE_FILE_CMD, filePath);
    this->execute(cmd);
}

QString KSS::getModuleFiles()
{
    this->execute(DIGEST_SCAN_GET_KERNEL_CMD);
    return m_processOutput;
}

void KSS::prohibitUnloading(bool prohibited, const QString &filePath)
{
}

QString KSS::getExecuteFiles()
{
    this->execute(DIGEST_SCAN_GET_EXECUTE_CMD);
    return m_processOutput;
}

void KSS::addFile(const QString &fileName, const QString &filePath, const QString &insertTime)
{
    auto cmd = QString("%1 -n %2 -p %3 -t '%4'").arg(ADD_FILE_CMD, fileName, filePath, insertTime);
    this->execute(cmd);
}

void KSS::removeFile(const QString &filePath)
{
    auto cmd = QString("%1 -p %2").arg(REMOVE_FILE_CMD, filePath);
    this->execute(cmd);
}

QString KSS::getFiles()
{
    this->execute(GET_FILES_CMD);
    return m_processOutput;
}

QString KSS::search(const QString &pathKey, const QString &fileList)
{
    QJsonDocument resultJsonDoc;
    QJsonArray jsonArr;

    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(fileList.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser kernel protected information failed: " << jsonError.errorString();
        return QString();
    }

    auto jsonModules = jsonDoc.object().value(KSS_JSON_KEY_DATA).toArray();

    for (auto module : jsonModules)
    {
        auto jsonMod = module.toObject();

        // 通过输入的pathKey，判断list中path字段是否包含pathKey
        if (jsonMod.value(KSS_JSON_KEY_DATA_PATH).toString().contains(pathKey))
        {
            jsonArr.push_back(jsonMod);
        }
    }
    resultJsonDoc.setArray(jsonArr);
    return QString(resultJsonDoc.toJson());
}

void KSS::onProcessExit(int exitCode, QProcess::ExitStatus exitStatus)
{
    KLOG_DEBUG() << "Command execution completed. exitcode = " << exitCode << "exitStatus = " << exitStatus;
    this->m_process->disconnect();

    auto standardOutput = this->m_process->readAllStandardOutput();

    KLOG_DEBUG() << "Execute the command to successfully output: " << standardOutput;
    m_processOutput = standardOutput;

    auto errordOutput = this->m_process->readAllStandardError();
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
    connect(this->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessExit(int, QProcess::ExitStatus)));
    m_process->waitForFinished();
}

void KSS::initDataResults()
{
    KLOG_INFO() << "Initialisation ok kss data is completed.";

    m_ini->setValue(KSS_INI_KEY, 1);
    emit this->initFinished();
}

void KSS::initData()
{
    RETURN_IF_TRUE(m_ini->value(KSS_INI_KEY).toInt() != 0)

    KLOG_INFO() << "Start kss initialisation.";
    this->execute(KSS_INIT_CMD);

    auto kssInitThread = QThread::create([this]
                                         {
                                             auto process = new QProcess(this);
                                             auto cmd = QString("%1 %2").arg(KSS_INIT_DATA_CMD, "/boot/vmlinuz-`uname -r`");
                                             KLOG_DEBUG() << "Start executing the command. cmd = " << cmd;
                                             process->start("bash", QStringList() << "-c" << cmd);
                                             process->waitForFinished(-1);
                                         });

    connect(kssInitThread, &QThread::finished, this, &KSS::initDataResults);
    connect(kssInitThread, &QThread::finished, kssInitThread, &QThread::deleteLater);

    kssInitThread->start();
}
}  // namespace KS
