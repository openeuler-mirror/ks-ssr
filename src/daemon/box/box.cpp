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

#include "box.h"
#include <pwd.h>
#include <qt5-log-i.h>
#include <QCryptographicHash>
#include <QDir>
#include <QRandomGenerator>
#include <QTime>
#include "config.h"
#include "include/sc-marcos.h"
#include "lib/base/crypto-helper.h"

namespace KS
{
#define GET_BOX_UID_BIT 6         // 随机生成box的标识符位数
#define GET_BOX_PASSPHRASE_BIT 8  // 随机生成口令的位数

Box::Box(const QString &name,
         const QString &password,
         uint userUID,
         const QString &boxId,
         QObject *parent) : QObject(parent),
                            m_name(name),
                            m_boxId(boxId),
                            m_password(password),
                            m_userUID(userUID)
{
    m_boxDao = new BoxDao;
    m_ecryptFS = new EcryptFS(this);

    this->init();
}

QString Box::getBoxID()
{
    return m_boxId;
}

QString Box::getBoxName()
{
    return m_name;
}

// 获取调用者用户名
QString Box::getUser()
{
    auto pwd = getpwuid(m_userUID);
    return pwd->pw_name;
}

uint Box::getUserUid()
{
    return m_userUID;
}

QString Box::getPassphrase()
{
    auto decryptPspr = CryptoHelper::aesDecrypt(m_boxDao->getBox(m_boxId).encryptPspr);
    return decryptPspr;
}

bool Box::retrievePassword(const QString &passphrase, const QString &newPassword)
{
    RETURN_VAL_IF_TRUE(getPassphrase() != passphrase, false)

    auto encryptPassword = CryptoHelper::aesEncrypt(newPassword);

    m_boxDao->modifyPasswd(m_boxId, encryptPassword);
    return true;
}

BoxRecord Box::getBoxInfo()
{
    return m_boxDao->getBox(m_boxId);
}

bool Box::delBox(const QString &inputPassword)
{
    // 密码认证
    auto boxInfo = getBoxInfo();
    auto decryptPassword = CryptoHelper::aesDecrypt(boxInfo.encryptpassword);

    if (inputPassword != decryptPassword)
    {
        KLOG_ERROR() << "Password error!";
        return false;
    }

    if (boxInfo.isMount)
    {
        KLOG_WARNING() << "The box has been mounted and cannot be deleted. uid = " << m_boxId;
        return false;
    }

    // 删除目录
    QString dirPath = QString("%1/%2%3").arg(SC_BOX_MOUNT_DATADIR, boxInfo.boxName, boxInfo.boxId);
    m_ecryptFS->rmBoxDir(dirPath);

    QString mountPath = QString("%1/%2").arg(SC_BOX_MOUNT_DIR, boxInfo.boxName);
    m_ecryptFS->rmBoxDir(mountPath);

    // 删除数据库
    m_boxDao->delBox(m_boxId);

    return true;
}

bool Box::isMount()
{
    auto boxInfo = getBoxInfo();

    return boxInfo.isMount;
}

bool Box::mount(const QString &inputPassword)
{
    // 密码认证
    auto boxInfo = getBoxInfo();
    auto decryptPassword = CryptoHelper::aesDecrypt(boxInfo.encryptpassword);

    if (inputPassword != decryptPassword)
    {
        KLOG_ERROR() << "Mount password error!";
        return false;
    }

    // 已挂载则返回
    if (boxInfo.isMount)
    {
        KLOG_WARNING() << "The box has been mounted and there is no need to repeat the operation. uid = " << m_boxId;
        return true;
    }

    //挂载
    auto decryptPspr = CryptoHelper::aesDecrypt(boxInfo.encryptPspr);  //Pspr
    auto decryptSig = CryptoHelper::aesDecrypt(boxInfo.encryptSig);

    auto mountPath = QString("%1/%2").arg(SC_BOX_MOUNT_DIR, boxInfo.boxName);
    m_ecryptFS->mkdirBoxDir(mountPath, getUser());

    auto mountObjectPath = QString("%1/%2%3").arg(SC_BOX_MOUNT_DATADIR, boxInfo.boxName, boxInfo.boxId);

    RETURN_VAL_IF_TRUE(!m_ecryptFS->decrypt(mountObjectPath, mountPath, decryptPspr, decryptSig), false)

    // 修改数据库中挂载状态
    m_boxDao->modifyMountStatus(m_boxId, true);
    return true;
}

void Box::umount()
{
    auto boxInfo = getBoxInfo();
    QString mountPath = QString("%1/%2").arg(SC_BOX_MOUNT_DIR, boxInfo.boxName);

    m_ecryptFS->encrypt(mountPath);
    // 修改数据库中挂载状态
    m_boxDao->modifyMountStatus(m_boxId, false);
}

bool Box::modifyBoxPassword(const QString &inputPassword, const QString &newPassword)
{
    auto boxInfo = getBoxInfo();
    auto decryptPassword = CryptoHelper::aesDecrypt(boxInfo.encryptpassword);
    //    auto decryptInputPassword = CryptoHelper::rsa_decrypt(m_rsaPrivateKey, m_password);
    if (inputPassword != decryptPassword)
    {
        KLOG_ERROR() << "Password error!";
        return false;
    }

    auto encryptPassword = CryptoHelper::aesEncrypt(newPassword);

    m_boxDao->modifyPasswd(m_boxId, encryptPassword);
    return true;
}

void Box::init()
{
    if (m_boxId.isEmpty())
    {
        QString passphrase = getRandStr(GET_BOX_PASSPHRASE_BIT);
        QString sig = m_ecryptFS->addPassphrase(passphrase);
        if (passphrase.isEmpty())
        {
            KLOG_ERROR() << "generate passphrase fail. check ecryptfs.ko is load!";
        }

        // aes加密处理 存入数据库
        auto encryptPassphrase = CryptoHelper::aesEncrypt(passphrase);
        auto encryptSig = CryptoHelper::aesEncrypt(sig.trimmed());
        auto encryptPasswd = CryptoHelper::aesEncrypt(m_password);

        // 创建随机uid 暂定为六位字符, 作为唯一标识符
        m_boxId = getRandBoxUid();

        // 插入数据库
        m_boxDao->addBox(m_name, m_boxId, false, encryptPasswd, encryptPassphrase,
                         encryptSig, m_userUID);
    }

    // 创建对应文件夹 未挂载box
    QDir dir(SC_BOX_MOUNT_DATADIR);
    if (!dir.mkdir(m_name + m_boxId))  // name+uid 命名 可区分不同用户下创建的相同文件夹名称
    {
        KLOG_WARNING() << "Failed to create folder. name = " << m_name << " uid = " << m_boxId;
    }

    // 在对应调用的用户根目录创建文件夹
    QString mountPath = QString("%1/%2").arg(SC_BOX_MOUNT_DIR, m_name);
    m_ecryptFS->mkdirBoxDir(mountPath, getUser());
}

// 生成6位不重复uid,作为box标识
QString Box::getRandBoxUid()
{
    QString uid;
    for (int i = 0; i < 5; i++)
    {
        uid = getRandStr(GET_BOX_UID_BIT);

        auto boxInfo = m_boxDao->getBox(uid);
        // 若存在则重新生成uid
        if (!boxInfo.boxId.isEmpty())
        {
            KLOG_WARNING() << "There is same uid. uid = " << uid;
            uid = "";
            continue;
        }
        else
        {
            break;
        }
    }

    if (uid.isEmpty())
    {
        KLOG_ERROR() << "Random uid cannot be generated!";
    }

    return uid;
}

// 生成随机字符串
QString Box::getRandStr(uint length)
{
    auto hash = QCryptographicHash::hash(QTime::currentTime().toString().toUtf8(), QCryptographicHash::Md5);
    QString md5 = hash.toHex();
    QString str;

    int num = 0;
    for (uint i = 0; i < length; ++i)
    {
        num = QRandomGenerator::global()->bounded(md5.length());
        str.insert(i, md5.at(num));
    }

    return str;
}
}  // namespace KS
