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
#include "include/ksc-marcos.h"

namespace KS
{
EcryptFS::EcryptFS(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(parent);
}

QString EcryptFS::addPassphrase(const QString &passphrase)
{
    RETURN_VAL_IF_TRUE(!isExistEcryptFSMod(), QString())
    QString cmd = QString("echo '%1' | %2").arg(passphrase, GENERATE_PASSPHRASE_CMD);
    execute(cmd);
    return m_processOutput;
}

QString EcryptFS::encrypt(const QString &umountPath)
{
    QString cmd = QString("umount %1").arg(umountPath);
    execute(cmd);

    return m_errorOutput;
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
    execute(cmd);
    RETURN_VAL_IF_TRUE(m_errorOutput.isEmpty(), true)

    return false;
}

bool EcryptFS::mkdirBoxDir(const QString &path, const QString &userName)
{
    QDir dir(path);
    RETURN_VAL_IF_TRUE(dir.exists(), true);
    // name+uid 命名 可区分不同用户下创建的相同文件夹名称
    if (!dir.mkpath(path))
    {
        KLOG_WARNING() << "Failed to mkdir folder. path = " << path;
        return false;
    }

    QString cmd = QString("chown %2:%2 %1").arg(path, userName);
    execute(cmd);
    return true;
}

void EcryptFS::rmBoxDir(const QString &path)
{
    QDir dir(path);
    if (!dir.removeRecursively())
    {
        KLOG_WARNING() << "Failed to remove folder. path = " << path;
    }
}

void EcryptFS::processExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    KLOG_DEBUG() << "Command execution completed. exitcode = " << exitCode << "exitStatus = " << exitStatus;
    m_process->disconnect();

    QByteArray standardOutput = m_process->readAllStandardOutput();

    KLOG_DEBUG() << "Execute the command to successfully output: " << standardOutput;
    m_processOutput = standardOutput;
    QByteArray errordOutput = m_process->readAllStandardError();
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
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processExited(int, QProcess::ExitStatus)));
    m_process->waitForFinished();
}

bool EcryptFS::isExistEcryptFSMod()
{
    QString cmd = QString("lsmod |grep ecryptfs");
    execute(cmd);
    return !m_processOutput.isEmpty();
}
}  // namespace KS
