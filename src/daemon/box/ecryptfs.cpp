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

#include "ecryptfs.h"
#include <qt5-log-i.h>

namespace KS
{
EcryptFS::EcryptFS(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(parent);
}

QString EcryptFS::generate_passphrase(const QString key)
{
    QString cmd = QString("echo '%1' |").arg(key) + GENERATE_PASSPHRASE_CMD;
    if (m_process->state() != QProcess::Running)
    {
        KLOG_DEBUG() << "cmd = " << cmd;
        m_process->start("bash", QStringList() << "-c" << cmd);
        connect(this->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessExit(int, QProcess::ExitStatus)));
        m_process->waitForFinished(1000);
    }
    return m_processOutput;
}

void EcryptFS::encrypt(const QString umountPath)
{
    QString cmd = QString("umount %1").arg(umountPath);
    if (m_process->state() != QProcess::Running)
    {
        KLOG_DEBUG() << "cmd = " << cmd;
        m_process->start("bash", QStringList() << "-c" << cmd);
        connect(this->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessExit(int, QProcess::ExitStatus)));
        m_process->waitForFinished(1000);
    }
}

bool EcryptFS::dcrypt(const QString mountObjectPath, const QString mountPath, const QString key, const QString passphrase)
{
    //    mkdir(mountPath);
    QString cmd = MOUNT_ECRYPTFS_CMD + QString(" %1 %2  -o passphrase_passwd=%3,ecryptfs_sig=%4,ecryptfs_fnek_sig=%5,key=passphrase,ecryptfs_cipher=aes,ecryptfs_key_bytes=16,ecryptfs_passthrough,ecryptfs_enable_filename_crypto=y,no_sig_cache")
                                           .arg(mountObjectPath, mountPath, key, passphrase, passphrase);
    if (m_process->state() != QProcess::Running)
    {
        KLOG_DEBUG() << "cmd = " << cmd;
        m_process->start("bash", QStringList() << "-c" << cmd);
        connect(this->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessExit(int, QProcess::ExitStatus)));
        m_process->waitForFinished(1000);
    }
    if (m_errorOutput == "")
        return true;
    else
        return false;
}

void EcryptFS::mkdirBoxDir(const QString path)
{
    QString cmd = QString("mkdir -p %1").arg(path);
    if (m_process->state() != QProcess::Running)
    {
        KLOG_DEBUG() << "cmd = " << cmd;
        m_process->start("bash", QStringList() << "-c" << cmd);
        connect(this->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessExit(int, QProcess::ExitStatus)));
        m_process->waitForFinished(1000);
    }
}

void EcryptFS::rmBoxDir(const QString path)
{
    QString cmd = QString("rm -r %1").arg(path);
    if (m_process->state() != QProcess::Running)
    {
        KLOG_DEBUG() << "cmd = " << cmd;
        m_process->start("bash", QStringList() << "-c" << cmd);
        connect(this->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessExit(int, QProcess::ExitStatus)));
        m_process->waitForFinished(1000);
    }
}

void EcryptFS::onProcessExit(int exitCode, QProcess::ExitStatus exitStatus)
{
    KLOG_DEBUG() << "exitcode = " << exitCode << "exitStatus = " << exitStatus;
    this->m_process->disconnect();

    QByteArray standardOutput = this->m_process->readAllStandardOutput();

    KLOG_DEBUG() << "Standard output: " << standardOutput;
    m_processOutput = standardOutput;

    QByteArray errordOutput = this->m_process->readAllStandardError();
    if (errordOutput != "")
        KLOG_DEBUG() << "Error output: " << errordOutput;
    m_errorOutput = errordOutput;
}
}  // namespace KS
