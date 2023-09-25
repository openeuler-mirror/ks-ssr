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
    explicit EcryptFS(QObject *parent = nullptr);
    virtual ~EcryptFS(){};

    // 通过口令生成密钥 ，返回passphrase
    QString generate_passphrase(const QString &key);
    // 解密 umount
    void encrypt(const QString &umountPath);
    // 解密：mount key:口令 passphrase：密钥
    bool dcrypt(const QString &mountObjectPath, const QString &mountPath, const QString &key, const QString &passphrase);
    void mkdirBoxDir(const QString &path);
    void rmBoxDir(const QString &path);

public Q_SLOTS:
    void onProcessExit(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_process;
    QString m_processOutput = "";
    QString m_errorOutput = "";
};
}  // namespace KS
#endif  // ECRYPTFS_H
