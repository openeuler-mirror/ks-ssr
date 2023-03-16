/**
 * @file          /ks-sc/src/daemon/box/box-manager.cpp
 * @brief
 * @author        chendingjian <chendingjian@kylinos.com>
 * @copyright (c) 2023 KylinSec. All rights reserved.
 */
#include "src/daemon/box/box-manager.h"
#include <pwd.h>
#include <QDBusConnection>
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

    m_ecryptFS = new EcryptFS(this);
    this->init();
}
BoxManager::~BoxManager()
{
}

// 生成随机字符串
QString getRandStr(uint length)
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
QString getRandBoxUid()
{
    QString uid = getRandStr(6);

    QSqlQuery query = BoxDao::getInstance()->findQuery(uid);
    // 若存在则重新生成uid
    if ("" != query.value(1).toString())
    {
        KLOG_DEBUG() << "There is same uid. uid = " << uid;
        getRandBoxUid();
    }

    return uid;
}

// 获取调用者用户名
QString getSendUserName(const uint &userUid)
{
    struct passwd *pwd;
    pwd = getpwuid(userUid);
    return pwd->pw_name;
}

QString BoxManager::CreateBox(const QString &name, const QString &password)
{
    try
    {
        // 获取调用者用户uid
        QDBusConnection conn = connection();
        QDBusMessage msg = message();
        KLOG_DEBUG() << "sender Uid = " << uint(conn.interface()->serviceUid(msg.service()).value());

        QString key = getRandStr(8);
        QString passphrase = m_ecryptFS->generate_passphrase(key);
        if ("" == passphrase)
        {
            KLOG_ERROR() << "generate passphrase fail. check ecryptfs.ko is load!";
            return "";
        }

        // 加密处理
        std::string encryptKey = CryptoHelper::rsa_encrypt(m_rSAPublicKey, key.toStdString());
        std::string encryptPassphrase = CryptoHelper::rsa_encrypt(m_rSAPublicKey, StrUtils::rtrim(passphrase.toStdString()));
        std::string encryptPassword = CryptoHelper::rsa_encrypt(m_rSAPublicKey, password.toStdString());

        KLOG_DEBUG() << "key = " << key << "passphrase = " << passphrase;
        KLOG_DEBUG() << "encryptPassword = " << QString::fromStdString(encryptPassword);

        // 创建随机uid 暂定为六位字符, 作为唯一标识符
        QString uid = getRandBoxUid();

        // 插入数据库
        BoxDao::getInstance()->addQuery(name, uid, false, QString::fromStdString(encryptPassword), QString::fromStdString(encryptKey),
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
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        return "";
    }
}

void BoxManager::DelBox(const QString &box_uid, const QString &password)
{
    try
    {
        // 密码认证
        QSqlQuery query = BoxDao::getInstance()->findQuery(box_uid);
        std::string decryptPassword = CryptoHelper::rsa_decrypt(m_rSAPrivateKey, query.value(3).toString().toStdString());

        if (password.toStdString() != decryptPassword)
        {
            KLOG_DEBUG() << "DelBox password error!";
            return;
        }

        if (QVariant(query.value(2).toString()).toBool())
        {
            KLOG_DEBUG() << "Is mounted. uid = " << box_uid;
            return;
        }

        // 删除目录
        QString dirPath = UNMOUNTED_DIR_CREAT_PATH "/" + query.value(0).toString() + query.value(1).toString();
        m_ecryptFS->rmBoxDir(dirPath);

        if (0 != query.value(6).toInt())
            dirPath = "~/box/" + query.value(0).toString();
        else
        {
            dirPath = QString("/home/%1/box/%2").arg(getSendUserName(query.value(6).toInt()), query.value(0).toString());
        }
        m_ecryptFS->rmBoxDir(dirPath);

        // 删除数据库
        BoxDao::getInstance()->delQuery(box_uid);

        emit BoxDeleted(box_uid);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
    }
}

QString BoxManager::GetBoxByUID(const QString &box_uid)
{
    try
    {
        QSqlQuery query = BoxDao::getInstance()->findQuery(box_uid);
        Json::Value values;
        values[BOX_NAME_KEY] = query.value(0).toString().toStdString();
        values[BOX_UID_KEY] = query.value(1).toString().toStdString();
        values[BOX_ISMOUNT_KEY] = query.value(2).toString().toStdString();
        values[BOX_PASSWD_KEY] = query.value(3).toString().toStdString();
        std::string boxInfor = StrUtils::json2str(values);

        return QString::fromStdString(boxInfor);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        return "";
    }
}

QString BoxManager::GetBoxs()
{
    try
    {
        QSqlQuery query(BoxDao::getInstanceDb());
        Json::Value values;
        int i = 0;

        QString cmd = "select * from notes;";
        if (!query.exec(cmd))
            KLOG_DEBUG() << "select error!";
        else
        {
            while (query.next())
            {
                // 暂只返回以下四个数据给前台
                values[i][BOX_NAME_KEY] = query.value(0).toString().toStdString();
                values[i][BOX_UID_KEY] = query.value(1).toString().toStdString();
                values[i][BOX_ISMOUNT_KEY] = query.value(2).toString().toStdString();
                values[i][BOX_PASSWD_KEY] = query.value(3).toString().toStdString();

                i++;
            }
        }

        std::string boxInfors = StrUtils::json2str(values);

        return QString::fromStdString(boxInfors);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        return "";
    }
}

bool BoxManager::IsMounted(const QString &box_uid)
{
    try
    {
        QSqlQuery query = BoxDao::getInstance()->findQuery(box_uid);

        return QVariant(query.value(2).toString()).toBool();
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        return false;
    }
}

void BoxManager::ModifyBoxPassword(const QString &box_uid, const QString &current_password, const QString &new_password)
{
    try
    {
        QSqlQuery query = BoxDao::getInstance()->findQuery(box_uid);
        std::string decryptPassword = CryptoHelper::rsa_decrypt(m_rSAPrivateKey, query.value(3).toString().toStdString());

        if (current_password.toStdString() != decryptPassword)
        {
            KLOG_DEBUG() << "ModifyBoxPassword password error!";
            return;
        }

        std::string encryptPassword = CryptoHelper::rsa_encrypt(m_rSAPublicKey, new_password.toStdString());
        BoxDao::getInstance()->ModifyQueryPasswd(box_uid, QString::fromStdString(encryptPassword));
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        return;
    }
}

// 解锁
bool BoxManager::Mount(const QString &box_uid, const QString &password)
{
    try
    {
        // 密码认证
        QSqlQuery query = BoxDao::getInstance()->findQuery(box_uid);
        std::string decryptPassword = CryptoHelper::rsa_decrypt(m_rSAPrivateKey, query.value(3).toString().toStdString());

        if (password.toStdString() != decryptPassword)
        {
            KLOG_DEBUG() << "Mount password error!";
            return false;
        }

        // 已挂载则返回
        if (QVariant(query.value(2).toString()).toBool())
        {
            KLOG_DEBUG() << "Is mounted. uid = " << box_uid;
            return true;
        }

        //挂载
        std::string decryptKey = CryptoHelper::rsa_decrypt(m_rSAPrivateKey, query.value(4).toString().toStdString());
        KLOG_DEBUG() << "decryptKeyPspr = " << QString::fromStdString(decryptKey);
        std::string decryptPspr = CryptoHelper::rsa_decrypt(m_rSAPrivateKey, query.value(5).toString().toStdString());  //Pspr
        KLOG_DEBUG() << "decryptKeyPspr = " << QString::fromStdString(decryptPspr);

        QString mountPath;
        if (0 != query.value(6).toInt())
            mountPath = "~/box/" + query.value(0).toString();
        else
        {
            mountPath = QString("/home/%1/box/%2").arg(getSendUserName(query.value(6).toInt()), query.value(0).toString());
        }

        m_ecryptFS->mkdirBoxDir(mountPath);

        if (m_ecryptFS->dcrypt(UNMOUNTED_DIR_CREAT_PATH "/" + query.value(0).toString() + query.value(1).toString(), mountPath,
                               QString::fromStdString(decryptKey), QString::fromStdString(decryptPspr)))
        {
            // 修改数据库中挂载状态
            BoxDao::getInstance()->ModifyQueryMountStatus(box_uid, true);
            emit BoxChanged(box_uid);
            return true;
        }
        else
            return false;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        return false;
    }
}

// 上锁
void BoxManager::UnMount(const QString &box_uid)
{
    try
    {
        QSqlQuery query = BoxDao::getInstance()->findQuery(box_uid);
        QString mountPath;
        if (0 != query.value(6).toInt())
            mountPath = "~/box/" + query.value(0).toString();
        else
        {
            mountPath = QString("/home/%1/box/%2").arg(getSendUserName(query.value(6).toInt()), query.value(0).toString());
        }

        m_ecryptFS->encrypt(mountPath);
        // 修改数据库中挂载状态
        BoxDao::getInstance()->ModifyQueryMountStatus(box_uid, false);
        emit BoxChanged(box_uid);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
    }
}

void BoxManager::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(SC_BOX_MANAGER_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }

    KS::CryptoHelper::generate_rsa_key(RSA_KEY_LENGTH, m_rSAPrivateKey, m_rSAPublicKey);
}

}  // namespace KS
