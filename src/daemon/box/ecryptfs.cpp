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
#include <QDir>
#include "include/sc-marcos.h"

namespace KS
{
EcryptFS::EcryptFS(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(parent);
}

QString EcryptFS::addPassphrase(const QString &passphrase)
{
    QString cmd = QString("echo '%1' | %2").arg(passphrase, GENERATE_PASSPHRASE_CMD);
    this->execute(cmd);
    return m_processOutput;
}

void EcryptFS::encrypt(const QString &umountPath)
{
    QString cmd = QString("umount %1").arg(umountPath);
    this->execute(cmd);
}

bool EcryptFS::decrypt(const QString &mountObjectPath,
                       const QString &mountPath,
                       const QString &passphrase,
                       const QString &sig)
{
    QString cmd = QString("%1 %2 %3  -o passphrase_passwd=%4,"
                          "ecryptfs_sig=%5,"
                          "ecryptfs_fnek_sig=%6,"
                          "key=passphrase,"
                          "ecryptfs_cipher=aes,"
                          "ecryptfs_key_bytes=16,"
                          "ecryptfs_passthrough,"
                          "ecryptfs_enable_filename_crypto=y,"
                          "no_sig_cache")
                      .arg(MOUNT_ECRYPTFS_CMD, mountObjectPath, mountPath, passphrase, sig, sig);
    this->execute(cmd);
    RETURN_VAL_IF_TRUE(m_errorOutput.isEmpty(), true)

    return false;
}

void EcryptFS::mkdirBoxDir(const QString &path, const QString &userName)
{
    QDir dir(path);
    if (!dir.mkpath(path))  // name+uid 命名 可区分不同用户下创建的相同文件夹名称
    {
        KLOG_WARNING() << "Failed to mkdir folder. path = " << path;
    }

    QString cmd = QString("chown %2:%2 %1").arg(path, userName);
    this->execute(cmd);
}

void EcryptFS::rmBoxDir(const QString &path)
{
    QDir dir(path);
    if (!dir.rmpath(path))
    {
        KLOG_WARNING() << "Failed to remove folder. path = " << path;
    }
}

void EcryptFS::onProcessExit(int exitCode, QProcess::ExitStatus exitStatus)
{
    KLOG_DEBUG() << "Command execution completed. exitcode = " << exitCode << "exitStatus = " << exitStatus;
    this->m_process->disconnect();

    QByteArray standardOutput = this->m_process->readAllStandardOutput();

    KLOG_DEBUG() << "Execute the command to successfully output: " << standardOutput;
    m_processOutput = standardOutput;
    QByteArray errordOutput = this->m_process->readAllStandardError();
    if (!errordOutput.isEmpty())
    {
        KLOG_ERROR() << "Execution command error output: " << errordOutput;
    }
    m_errorOutput = errordOutput;
}

void EcryptFS::execute(const QString &cmd)
{
    KLOG_DEBUG() << "Start executing the command. cmd = " << cmd;
    m_process->start("bash", QStringList() << "-c" << cmd);
    connect(this->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessExit(int, QProcess::ExitStatus)));
    m_process->waitForFinished();
}
}  // namespace KS