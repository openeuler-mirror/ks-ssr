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

#include "box.h"
#include <pwd.h>
#include <qt5-log-i.h>
#include <QCryptographicHash>
#include <QDir>
#include <QRandomGenerator>
#include <QTime>
#include "config.h"
#include "include/ssr-error-i.h"
#include "include/ssr-marcos.h"
#include "lib/base/crypto-helper.h"
#include "ssr-error-i.h"

namespace KS
{
namespace PrivateBox
{
#define GET_BOX_UID_BIT 6         // 随机生成box的标识符位数
#define GET_BOX_PASSPHRASE_BIT 8  // 随机生成口令的位数

Box *Box::create(const QString &name,
                 const QString &password,
                 uint userUID,
                 int &errorCode,
                 const QString &boxID,
                 QObject *parent)
{
    auto box = new Box(name, password, userUID, boxID, parent);
    if (!box->init(errorCode))
    {
        delete box;
        box = nullptr;
    }
    return box;
}

QString Box::getBoxID()
{
    return m_boxID;
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
    auto decryptPassphrase = CryptoHelper::aesDecrypt(m_boxDao->getBox(m_boxID).encryptPassphrase);
    return decryptPassphrase;
}

QString Box::retrievePassword(const QString &passphrase)
{
    RETURN_VAL_IF_TRUE(getPassphrase() != passphrase, QString())

    return CryptoHelper::aesDecrypt(m_boxDao->getBox(m_boxID).encryptpassword);
}

BoxRecord Box::getBoxInfo()
{
    return m_boxDao->getBox(m_boxID);
}

int Box::delBox(const QString &currentPassword)
{
    int errorEode = (int)SSRErrorCode::SUCCESS;
    // 密码认证
    auto boxInfo = getBoxInfo();
    auto decryptPassword = CryptoHelper::aesDecrypt(boxInfo.encryptpassword);

    if (currentPassword != decryptPassword)
    {
        KLOG_WARNING() << "Password error!";
        errorEode = (int)SSRErrorCode::ERROR_BM_INPUT_PASSWORD_ERROR;
        return errorEode;
    }

    if (boxInfo.mounted)
    {
        // 如果已挂载则先unmount #5047
        KLOG_DEBUG() << "The box has been mounted and cannot be deleted. uid = " << m_boxID;

        errorEode = umount();
        if (errorEode != (int)SSRErrorCode::SUCCESS)
        {
            return errorEode;
        }
        // return false;
    }

    // 删除目录
    QString dirPath = QString("%1/%2%3").arg(SSR_BOX_MOUNT_DATADIR, boxInfo.boxName, boxInfo.boxID);
    m_ecryptFS->rmBoxDir(dirPath);

    QString mountPath = QString("%1/%2").arg(SSR_BOX_MOUNT_DIR, boxInfo.boxName);
    m_ecryptFS->rmBoxDir(mountPath);

    // 删除数据库
    m_boxDao->delBox(m_boxID);

    return errorEode;
}

bool Box::mounted()
{
    auto boxInfo = getBoxInfo();

    return boxInfo.mounted;
}

int Box::mount(const QString &currentPassword)
{
    // 密码认证
    auto boxInfo = getBoxInfo();
    auto decryptPassword = CryptoHelper::aesDecrypt(boxInfo.encryptpassword);

    if (currentPassword != decryptPassword)
    {
        KLOG_WARNING() << "Mount password error!";
        return (int)SSRErrorCode::ERROR_BM_INPUT_PASSWORD_ERROR;
    }

    // 已挂载则返回
    if (boxInfo.mounted)
    {
        KLOG_WARNING() << "The box has been mounted and there is no need to repeat the operation. uid = " << m_boxID;
        return (int)SSRErrorCode::SUCCESS;
    }

    // 挂载
    auto decryptPspr = CryptoHelper::aesDecrypt(boxInfo.encryptPassphrase);  //Pspr
    auto decryptSig = CryptoHelper::aesDecrypt(boxInfo.encryptSig);

    // 在对应调用的用户根目录创建文件夹
    auto mountPath = QString("%1/%2").arg(SSR_BOX_MOUNT_DIR, boxInfo.boxName);
    if (!m_ecryptFS->mkdirBoxDir(mountPath, getUser()))
    {
        return (int)SSRErrorCode::ERROR_BM_INTERNAL_ERRORS;
    }

    auto mountObjectPath = QString("%1/%2%3").arg(SSR_BOX_MOUNT_DATADIR, boxInfo.boxName, boxInfo.boxID);
    if (!m_ecryptFS->decrypt(mountObjectPath, mountPath, decryptPspr, decryptSig))
    {
        return (int)SSRErrorCode::ERROR_BM_INTERNAL_ERRORS;
    }

    // 修改数据库中挂载状态
    if (!m_boxDao->modifyMountStatus(m_boxID, true))
    {
        return (int)SSRErrorCode::ERROR_BM_INTERNAL_ERRORS;
    }
    return (int)SSRErrorCode::SUCCESS;
}

int Box::umount(bool isForce)
{
    auto boxInfo = getBoxInfo();
    auto mountPath = QString("%1/%2").arg(SSR_BOX_MOUNT_DIR, boxInfo.boxName);

    // 修改数据库中挂载状态
    if (!m_boxDao->modifyMountStatus(m_boxID, false))
    {
        return (int)SSRErrorCode::ERROR_BM_INTERNAL_ERRORS;
    }

    if (!m_ecryptFS->encrypt(mountPath, isForce).isEmpty())
    {
        // 挂载失败，将数据库中的数据改回去
        m_boxDao->modifyMountStatus(m_boxID, true);
        return (int)SSRErrorCode::ERROR_BM_UMOUNT_FAIL;
    }

    m_ecryptFS->rmBoxDir(mountPath);
    return (int)SSRErrorCode::SUCCESS;
}

bool Box::modifyBoxPassword(const QString &currentPassword, const QString &newPassword)
{
    auto boxInfo = getBoxInfo();
    auto decryptPassword = CryptoHelper::aesDecrypt(boxInfo.encryptpassword);
    if (currentPassword != decryptPassword)
    {
        KLOG_WARNING() << "Password error!";
        return false;
    }

    auto encryptPassword = CryptoHelper::aesEncrypt(newPassword);

    m_boxDao->modifyPasswd(m_boxID, encryptPassword);
    return true;
}

Box::Box(const QString &name,
         const QString &password,
         uint userUID,
         const QString &boxID,
         QObject *parent) : QObject(parent),
                            m_name(name),
                            m_boxID(boxID),
                            m_password(password),
                            m_userUID(userUID)
{
    m_boxDao = new BoxDao;
    m_ecryptFS = new EcryptFS(this);
}

bool Box::init(int &errorCode)
{
    if (!addToDao())
    {
        errorCode = (int)SSRErrorCode::ERROR_BM_MOUDLE_UNLOAD;
        return false;
    }

    if (!mkdirSourceDir())
    {
        errorCode = (int)SSRErrorCode::ERROR_BM_MKDIR_DATA_DIR_FAILED;
        return false;
    }
    return true;
}

bool Box::addToDao()
{
    // 保险箱已存在数据库中，返回true
    RETURN_VAL_IF_TRUE(!m_boxDao->getBox(m_boxID).boxName.isEmpty(), true);

    auto passphrase = getRandStr(GET_BOX_PASSPHRASE_BIT);
    auto sig = m_ecryptFS->addPassphrase(passphrase);
    if (sig.isEmpty())
    {
        KLOG_WARNING() << "generate passphrase fail. check ecryptfs.ko is load!";
        return false;
    }

    // aes加密处理 存入数据库
    auto encryptPassphrase = CryptoHelper::aesEncrypt(passphrase);
    auto encryptSig = CryptoHelper::aesEncrypt(sig.trimmed());
    auto encryptPasswd = CryptoHelper::aesEncrypt(m_password);

    // 创建随机uid 暂定为六位字符, 作为唯一标识符
    m_boxID = getRandBoxUid();

    // 插入数据库
    m_boxDao->addBox(m_name,
                     m_boxID,
                     false,
                     encryptPasswd,
                     encryptPassphrase,
                     encryptSig,
                     m_userUID);
    return true;
}

void Box::clearMountStatus()
{
    // 获取系统中保险箱实际挂载状态
    auto process = new QProcess(this);
    process->start("bash", QStringList() << "-c"
                                         << "df -h");
    process->waitForFinished();
    auto output = QString(process->readAllStandardOutput()).remove(QRegExp("\\s"));
    m_boxDao->modifyMountStatus(m_boxID, output.contains(m_name + m_boxID));
    // 处理终端占用目录，且在挂载状态下重启的情况，重启后实际上已经取消挂载了，但目录依然存在
    if (!mounted())
    {
        auto mountPath = QString("%1/%2").arg(SSR_BOX_MOUNT_DIR, m_name);
        m_ecryptFS->rmBoxDir(mountPath);
    }
    process->deleteLater();
}

bool Box::mkdirSourceDir()
{
    // 创建对应文件夹 未挂载box
    // name+uid 命名 可区分不同用户下创建的相同文件夹名称
    auto dataPath = QString("%1/%2").arg(SSR_BOX_MOUNT_DATADIR, m_name + m_boxID);
    auto result = m_ecryptFS->mkdirBoxDir(dataPath, getUser());
    // 创建时失败需清除目录与数据库
    if (!result)
    {
        // 删除目录
        QString dirPath = QString("%1/%2%3").arg(SSR_BOX_MOUNT_DATADIR, m_name, m_boxID);
        m_ecryptFS->rmBoxDir(dirPath);
        // 删除数据库
        m_boxDao->delBox(m_boxID);
    }

    return result;
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
        if (!boxInfo.boxID.isEmpty())
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

    for (uint i = 0; i < length; ++i)
    {
        int num = QRandomGenerator::global()->bounded(md5.length());
        str.insert(i, md5.at(num));
    }

    return str;
}
}  // namespace Box
}  // namespace KS
