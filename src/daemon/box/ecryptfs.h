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

#ifndef ECRYPTFS_H
#define ECRYPTFS_H

#include <ecryptfs.h>
#include <QProcess>

namespace KS
{
#define GENERATE_PASSPHRASE_CMD "ecryptfs-add-passphrase |grep -v Passphrase |cut -d[ -f 2 |cut -d] -f 1"
#define MOUNT_ECRYPTFS_CMD "mount -t ecryptfs"

class EcryptFS : public QObject
{
    Q_OBJECT
public:
    EcryptFS(QObject *parent = nullptr);
    virtual ~EcryptFS(){};

    // 添加口令 ，返回ecryptfs_sig
    QString addPassphrase(const QString &passphrase);
    // 解密 umount
    void encrypt(const QString &umountPath);
    /*
     * 解密：mount
     * @passphrase:口令
     * @sig：指定装载范围的身份验证令牌的签名，在执行装载之前，身份验证令牌必须位于内核密钥环中。
     */
    bool decrypt(const QString &mountObjectPath,
                 const QString &mountPath,
                 const QString &passphrase,
                 const QString &sig);
    void mkdirBoxDir(const QString &path, const QString &userName);
    void rmBoxDir(const QString &path);

public Q_SLOTS:
    void onProcessExit(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void execute(const QString &cmd);

private:
    QProcess *m_process;
    QString m_processOutput;
    QString m_errorOutput;
};
}  // namespace KS
#endif  // ECRYPTFS_H
