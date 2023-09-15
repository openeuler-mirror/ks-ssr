/**
 * @file          /ks-sc/src/daemon/box/ecryptfs.h
 * @brief         
 * @author        chendingjian <chendingjian@kylinos.com>
 * @copyright (c) 2023 KylinSec. All rights reserved.
 */
#ifndef ECRYPTFS_H
#define ECRYPTFS_H

#include <ecryptfs.h>
#include <QProcess>
#include "lib/base/base.h"

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

    QString generate_passphrase(const QString key);                                                                    // 通过口令生成密钥 ，返回passphrase
    void encrypt(const QString umountPath);                                                                            // 解密 umount
    bool dcrypt(const QString mountObjectPath, const QString mountPath, const QString key, const QString passphrase);  // 解密：mount key:口令 passphrase：密钥
    void mkdirBoxDir(const QString path);
    void rmBoxDir(const QString path);

public Q_SLOTS:
    void onProcessExit(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_process;
    QString m_processOutput = "";
    QString m_errorOutput = "";
};
}  // namespace KS
#endif  // ECRYPTFS_H
