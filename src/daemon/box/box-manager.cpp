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

#include "src/daemon/box/box-manager.h"
#include <pwd.h>
#include <qt5-log-i.h>
#include <QDBusConnection>
#include "config.h"
#include "include/sc-i.h"
#include "lib/base/crypto-helper.h"
#include "lib/base/str-utils.h"
#include "src/daemon/box/box-dao.h"
#include "src/daemon/box_manager_adaptor.h"

namespace KS
{
#define RSA_KEY_LENGTH 512
#define UNMOUNTED_DIR_CREAT_PATH SC_INSTALL_DATADIR "/box"

#define BOX_NAME_KEY "name"
#define BOX_UID_KEY "uid"
#define BOX_ISMOUNT_KEY "isMount"
#define BOX_PASSWD_KEY "encryptPassword"

BoxManager::BoxManager(QObject *parent) : QObject(parent)
{
    this->m_dbusAdaptor = new BoxManagerAdaptor(this);
    m_boxDao = new BoxDao;

    m_ecryptFS = new EcryptFS(this);
    this->init();
}
BoxManager::~BoxManager()
{
}

// 生成随机字符串
QString BoxManager::getRandStr(uint length)
{
    const char ch[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFJHIJKLMNOPQRSTUVWXYZ";
    int size = sizeof(ch);
    char *str = new char[length + 1];

    int num = 0;
    for (uint i = 0; i < length; ++i)
    {
        num = QRandomGenerator::global()->bounded(size);
        str[i] = ch[num];
    }

    return QString(str);
}

// 生成6位不重复uid,作为box标识
QString BoxManager::getRandBoxUid()
{
    QString uid = getRandStr(6);

    BoxInfo boxInfo = m_boxDao->getBox(uid);
    // 若存在则重新生成uid
    if ("" != boxInfo.boxId)
    {
        KLOG_DEBUG() << "There is same uid. uid = " << uid;
        getRandBoxUid();
    }

    return uid;
}

// 获取调用者用户名
QString BoxManager::getSendUserName(const uint &userUid)
{
    struct passwd *pwd;
    pwd = getpwuid(userUid);
    return pwd->pw_name;
}

QString BoxManager::CreateBox(const QString &name, const QString &password)
{
    // 获取调用者用户uid
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    KLOG_DEBUG() << " The caller uid of the new box is equal to " << uint(conn.interface()->serviceUid(msg.service()).value());

    QString key = getRandStr(8);
    QString passphrase = m_ecryptFS->generate_passphrase(key);
    if ("" == passphrase)
    {
        KLOG_ERROR() << "generate passphrase fail. check ecryptfs.ko is load!";
        return "";
    }

    // aes加密处理 存入数据库
    std::string encryptKey = CryptoHelper::aes_encrypt(key.toStdString());
    std::string encryptPassphrase = CryptoHelper::aes_encrypt(StrUtils::rtrim(passphrase.toStdString()));
    // passwd需先rsa解密后aes加密
    std::string decryptPasswd = CryptoHelper::rsa_decrypt(this->m_rSAPrivateKey.toStdString(), password.toStdString());
    std::string encryptPasswd = CryptoHelper::aes_encrypt(decryptPasswd);

    // 创建随机uid 暂定为六位字符, 作为唯一标识符
    QString uid = getRandBoxUid();

    // 插入数据库
    m_boxDao->addBox(name, uid, false, QString::fromStdString(encryptPasswd), QString::fromStdString(encryptKey),
                     QString::fromStdString(encryptPassphrase), uint(conn.interface()->serviceUid(msg.service()).value()));

    // 创建对应文件夹 未挂载box
    QDir dir(UNMOUNTED_DIR_CREAT_PATH);
    if (!dir.mkdir(name + uid))  // name+uid 命名 可区分不同用户下创建的相同文件夹名称
        KLOG_DEBUG() << "mkdir fail. name = " << name << " uid = " << uid;

    // 在对应调用的用户根目录创建文件夹
    if (0 == uint(conn.interface()->serviceUid(msg.service()).value()))
    {
        m_ecryptFS->mkdirBoxDir("~/box");
    }
    else
    {
        QString userName = getSendUserName(uint(conn.interface()->serviceUid(msg.service()).value()));
        m_ecryptFS->mkdirBoxDir(QString("/home/%1/box").arg(userName));
    }
    emit BoxAdded(uid);
    return uid;
}

bool BoxManager::DelBox(const QString &box_uid, const QString &password)
{
    // 密码认证
    BoxInfo boxInfo = m_boxDao->getBox(box_uid);
    std::string decryptPassword = CryptoHelper::aes_decrypt(boxInfo.encryptpassword.toStdString());
    std::string decryptInputPassword = CryptoHelper::rsa_decrypt(m_rSAPrivateKey.toStdString(), password.toStdString());

    if (decryptInputPassword != decryptPassword)
    {
        KLOG_ERROR() << "Password error!";
        return false;
    }

    if (boxInfo.isMount)
    {
        KLOG_DEBUG() << "Is mounted. uid = " << box_uid;
        return false;
    }

    // 删除目录
    QString dirPath = UNMOUNTED_DIR_CREAT_PATH "/" + boxInfo.boxName + boxInfo.boxId;
    m_ecryptFS->rmBoxDir(dirPath);

    if (0 == boxInfo.senderUserUid)
        dirPath = "~/box/" + boxInfo.boxName;
    else
    {
        dirPath = QString("/home/%1/box/%2").arg(getSendUserName(boxInfo.senderUserUid), boxInfo.boxName);
    }
    m_ecryptFS->rmBoxDir(dirPath);

    // 删除数据库
    m_boxDao->delBox(box_uid);

    emit BoxDeleted(box_uid);
    return true;
}

QString BoxManager::GetBoxByUID(const QString &box_uid)
{
    BoxInfo boxInfo = m_boxDao->getBox(box_uid);
    Json::Value values;
    values[BOX_NAME_KEY] = boxInfo.boxName.toStdString();
    values[BOX_UID_KEY] = boxInfo.boxId.toStdString();
    values[BOX_ISMOUNT_KEY] = QString::number(boxInfo.isMount).toStdString();
    values[BOX_PASSWD_KEY] = boxInfo.encryptpassword.toStdString();
    std::string boxInfor = StrUtils::json2str(values);

    return QString::fromStdString(boxInfor);
}

QString BoxManager::GetBoxs()
{
    // 获取调用者用户uid
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    QList<BoxInfo> boxInfoList = m_boxDao->getBoxs();
    Json::Value values;
    int i = 0;
    for (BoxInfo boxInfo : boxInfoList)
    {
        // 调用者uid检测，不属于调用者创建的box不返回给前台
        if (conn.interface()->serviceUid(msg.service()).value() != uint(boxInfo.senderUserUid))
        {
            i++;
            continue;
        }
        // 暂只返回以下四个数据给前台
        values[i][BOX_NAME_KEY] = boxInfo.boxName.toStdString();
        values[i][BOX_UID_KEY] = boxInfo.boxId.toStdString();
        values[i][BOX_ISMOUNT_KEY] = QString::number(boxInfo.isMount).toStdString();
        values[i][BOX_PASSWD_KEY] = boxInfo.encryptpassword.toStdString();
        i++;
    }

    std::string boxInfors = StrUtils::json2str(values);

    return QString::fromStdString(boxInfors);
}

bool BoxManager::IsMounted(const QString &box_uid)
{
    BoxInfo boxInfo = m_boxDao->getBox(box_uid);

    return boxInfo.isMount;
}

bool BoxManager::ModifyBoxPassword(const QString &box_uid, const QString &current_password, const QString &new_password)
{
    BoxInfo boxInfo = m_boxDao->getBox(box_uid);
    std::string decryptPassword = CryptoHelper::aes_decrypt(boxInfo.encryptpassword.toStdString());
    std::string decryptInputPassword = CryptoHelper::rsa_decrypt(m_rSAPrivateKey.toStdString(), current_password.toStdString());
    if (decryptInputPassword != decryptPassword)
    {
        KLOG_DEBUG() << "Password error!";
        return false;
    }

    //        std::string encryptPassword = CryptoHelper::rsa_encrypt(m_rSAPublicKey, new_password.toStdString());
    m_boxDao->modifyPasswd(box_uid, new_password);
    return true;
}

// 解锁
bool BoxManager::Mount(const QString &box_uid, const QString &password)
{
    // 密码认证
    BoxInfo boxInfo = m_boxDao->getBox(box_uid);
    std::string decryptPassword = CryptoHelper::aes_decrypt(boxInfo.encryptpassword.toStdString());
    std::string decryptInputPassword = CryptoHelper::rsa_decrypt(m_rSAPrivateKey.toStdString(), password.toStdString());

    if (decryptInputPassword != decryptPassword)
    {
        KLOG_DEBUG() << "Mount password error!";
        return false;
    }

    // 已挂载则返回
    if (boxInfo.isMount)
    {
        KLOG_DEBUG() << "Is mounted. uid = " << box_uid;
        return true;
    }

    //挂载
    std::string decryptKey = CryptoHelper::aes_decrypt(boxInfo.encryptKey.toStdString());
    std::string decryptPspr = CryptoHelper::aes_decrypt(boxInfo.encryptPspr.toStdString());  //Pspr

    QString mountPath;
    if (0 == boxInfo.senderUserUid)
        mountPath = "~/box/" + boxInfo.boxName;
    else
    {
        mountPath = QString("/home/%1/box/%2").arg(getSendUserName(boxInfo.senderUserUid), boxInfo.boxName);
    }

    m_ecryptFS->mkdirBoxDir(mountPath);

    if (m_ecryptFS->dcrypt(UNMOUNTED_DIR_CREAT_PATH "/" + boxInfo.boxName + boxInfo.boxId, mountPath,
                           QString::fromStdString(decryptKey), QString::fromStdString(decryptPspr)))
    {
        // 修改数据库中挂载状态
        m_boxDao->modifyMountStatus(box_uid, true);
        emit BoxChanged(box_uid);
        return true;
    }
    else
        return false;
}

// 上锁
void BoxManager::UnMount(const QString &box_uid)
{
    BoxInfo boxInfo = m_boxDao->getBox(box_uid);
    QString mountPath;
    if (0 == boxInfo.senderUserUid)
        mountPath = "~/box/" + boxInfo.boxName;
    else
    {
        mountPath = QString("/home/%1/box/%2").arg(getSendUserName(boxInfo.senderUserUid), boxInfo.boxName);
    }

    m_ecryptFS->encrypt(mountPath);
    // 修改数据库中挂载状态
    m_boxDao->modifyMountStatus(box_uid, false);
    emit BoxChanged(box_uid);
}

void BoxManager::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(SC_BOX_MANAGER_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }

    std::string rsaPrivateKey;
    std::string rsaPublicKey;
    KS::CryptoHelper::generate_rsa_key(RSA_KEY_LENGTH, rsaPrivateKey, rsaPublicKey);
    m_rSAPrivateKey = QString::fromStdString(rsaPrivateKey);
    m_rSAPublicKey = QString::fromStdString(rsaPublicKey);
}
}  // namespace KS
