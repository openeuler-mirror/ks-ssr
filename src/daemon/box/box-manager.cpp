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
#include "include/ksc-error-i.h"
#include "include/ksc-i.h"
#include "include/ksc-marcos.h"
#include "lib/base/crypto-helper.h"
#include "lib/base/error.h"
#include "src/daemon/box/box-dao.h"
#include "src/daemon/box_manager_adaptor.h"

namespace KS
{
#define RSA_KEY_LENGTH 512

#define BOX_NAME_KEY "name"
#define BOX_UID_KEY "uid"
#define BOX_ISMOUNT_KEY "mounted"

BoxManager *BoxManager::m_instance = nullptr;
void BoxManager::globalInit(QObject *parent)
{
    m_instance = new BoxManager(parent);
}

void BoxManager::globalDeinit()
{
    if (m_instance)
    {
        delete m_instance;
    }
}

QString BoxManager::CreateBox(const QString &name, const QString &password, QString &passphrase)
{
    RETURN_VAL_DBUS_ERROR_IF_TRUE(name.isEmpty() || password.isEmpty(),
                                  QString(),
                                  KSCErrorCode::ERROR_COMMON_INVALID_ARGS,
                                  message())

    for (auto iterator = m_boxs.begin(); iterator != m_boxs.end(); iterator++)
    {
        auto box = iterator.value();
        RETURN_VAL_DBUS_ERROR_IF_TRUE(box->getUserUid() == getSenderUid() && box->getBoxName() == name,
                                      QString(),
                                      KSCErrorCode::ERROR_BM_REPEATED_NAME,
                                      message())
    }

    auto decryptPasswd = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, password);
    auto box = new Box(name, decryptPasswd, getSenderUid());
    // 模块未加载
    if (box->getPassphrase().isEmpty())
    {
        sendErrorReply(QDBusError::Failed, KSC_ERROR2STR(KSCErrorCode::ERROR_BM_MOUDLE_UNLOAD));
        KLOG_WARNING() << "The kernel module is not loaded.";
        return QString();
    }

    m_boxs.insert(box->getBoxID(), box);
    passphrase = box->getPassphrase();

    return box->getBoxID();
}

void BoxManager::DelBox(const QString &boxID, const QString &password)
{
    auto box = m_boxs.value(boxID);
    RETURN_DBUS_ERROR_IF_TRUE(!box,
                              KSCErrorCode::ERROR_BM_NOT_FOUND,
                              message())

    auto decryptedPassword = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, password);
    RETURN_DBUS_ERROR_IF_TRUE(!box->delBox(decryptedPassword),
                              KSCErrorCode::ERROR_BM_DELETE_FAILED,
                              message())

    m_boxs.remove(boxID);
    emit BoxDeleted(boxID);
}

QString BoxManager::GetBoxByUID(const QString &boxID)
{
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;
    auto box = m_boxs.value(boxID);
    RETURN_VAL_DBUS_ERROR_IF_TRUE(!box,
                                  QString(),
                                  KSCErrorCode::ERROR_BM_NOT_FOUND,
                                  message())

    jsonObj = QJsonObject{
        {BOX_NAME_KEY, box->getBoxName()},
        {BOX_UID_KEY, boxID},
        {BOX_ISMOUNT_KEY, box->mounted()}};
    jsonDoc.setObject(jsonObj);

    return QString(jsonDoc.toJson());
}

QString BoxManager::GetBoxs()
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArr;

    for (auto iterator = m_boxs.begin(); iterator != m_boxs.end(); iterator++)
    {
        auto box = iterator.value();
        // 调用者uid检测，不属于调用者创建的box不返回给前台
        if (getSenderUid() != box->getUserUid())
        {
            continue;
        }
        // 暂只返回以下三个数据给前台
        QJsonObject jsonObj{
            {BOX_NAME_KEY, box->getBoxName()},
            {BOX_UID_KEY, box->getBoxID()},
            {BOX_ISMOUNT_KEY, box->mounted()}};

        jsonArr.push_back(jsonObj);
    }

    jsonDoc.setArray(jsonArr);
    return QString(jsonDoc.toJson());
}

bool BoxManager::IsMounted(const QString &boxID)
{
    auto box = m_boxs.value(boxID);
    RETURN_VAL_DBUS_ERROR_IF_TRUE(!box,
                                  false,
                                  KSCErrorCode::ERROR_BM_NOT_FOUND,
                                  message())

    return box->mounted();
}

void BoxManager::ModifyBoxPassword(const QString &boxID,
                                   const QString &currentPassword,
                                   const QString &newPassword)
{
    RETURN_DBUS_ERROR_IF_TRUE(boxID.isEmpty() || currentPassword.isEmpty() || newPassword.isEmpty(),
                              KSCErrorCode::ERROR_COMMON_INVALID_ARGS,
                              message())

    auto decryptedPassword = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, currentPassword);
    auto decryptNewPassword = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, newPassword);
    RETURN_DBUS_ERROR_IF_TRUE(decryptedPassword == decryptNewPassword,
                              KSCErrorCode::ERROR_BM_SETTINGS_SAME_PASSWORD,
                              message())

    auto box = m_boxs.value(boxID);
    RETURN_DBUS_ERROR_IF_TRUE(!box,
                              KSCErrorCode::ERROR_BM_NOT_FOUND,
                              message())
    RETURN_DBUS_ERROR_IF_TRUE(!box->modifyBoxPassword(decryptedPassword, decryptNewPassword),
                              KSCErrorCode::ERROR_BM_MODIFY_PASSWORD_FAILED,
                              message())
}

// 解锁
void BoxManager::Mount(const QString &boxID, const QString &password)
{
    auto decryptedPassword = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, password);
    auto box = m_boxs.value(boxID);
    RETURN_DBUS_ERROR_IF_TRUE(!box,
                              KSCErrorCode::ERROR_BM_NOT_FOUND,
                              message())

    RETURN_IF_TRUE(box->getBoxID() != boxID)
    RETURN_DBUS_ERROR_IF_TRUE(!box->mount(decryptedPassword),
                              KSCErrorCode::ERROR_BM_INPUT_PASSWORD_ERROR,
                              message())

    emit BoxChanged(boxID);
}

QString BoxManager::RetrieveBoxPassword(const QString &boxID, const QString &passphrase)
{
    RETURN_VAL_DBUS_ERROR_IF_TRUE(passphrase.isEmpty(),
                                  QString(),
                                  KSCErrorCode::ERROR_COMMON_INVALID_ARGS,
                                  message())

    auto decryptPassphrase = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, passphrase);
    auto box = m_boxs.value(boxID);
    RETURN_VAL_DBUS_ERROR_IF_TRUE(!box,
                                  QString(),
                                  KSCErrorCode::ERROR_BM_NOT_FOUND,
                                  message())

    auto password = box->retrievePassword(decryptPassphrase);
    // 口令错误
    RETURN_VAL_DBUS_ERROR_IF_TRUE(password.isEmpty(),
                                  QString(),
                                  KSCErrorCode::ERROR_BM_INPUT_PASSPHRASE_ERROR,
                                  message())

    return password;
}

// 上锁
void BoxManager::UnMount(const QString &boxID)
{
    auto box = m_boxs.value(boxID);
    RETURN_DBUS_ERROR_IF_TRUE(!box,
                              KSCErrorCode::ERROR_BM_NOT_FOUND,
                              message())

    RETURN_IF_TRUE(box->getBoxID() != boxID)

    if (!box->umount())
    {
        sendErrorReply(QDBusError::Failed, KSC_ERROR2STR(KSCErrorCode::ERROR_BM_UMOUNT_FAIL));
        return;
    }

    emit BoxChanged(boxID);
}

BoxManager::BoxManager(QObject *parent) : QObject(parent)
{
    m_dbusAdaptor = new BoxManagerAdaptor(this);

    init();
}
BoxManager::~BoxManager()
{
}

void BoxManager::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(KSC_BOX_MANAGER_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }

    KS::CryptoHelper::generateRsaKey(RSA_KEY_LENGTH, m_rsaPrivateKey, m_rsaPublicKey);

    m_boxs.clear();
    BoxDao *boxDao = new BoxDao;
    auto boxInfoList = boxDao->getBoxs();
    for (auto boxInfo : boxInfoList)
    {
        auto decryptPassword = CryptoHelper::aesDecrypt(boxInfo.encryptpassword);
        Box *box = new Box(boxInfo.boxName, decryptPassword, boxInfo.userUID, boxInfo.boxID);
        m_boxs.insert(boxInfo.boxID, box);
    }
}

uint BoxManager::getSenderUid()
{
    // 获取调用者用户uid
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    return uint(conn.interface()->serviceUid(msg.service()).value());
}
}  // namespace KS
