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
#include "include/sc-marcos.h"
#include "lib/base/crypto-helper.h"
#include "src/daemon/box/box-dao.h"
#include "src/daemon/box_manager_adaptor.h"

namespace KS
{
#define RSA_KEY_LENGTH 512

#define BOX_NAME_KEY "name"
#define BOX_UID_KEY "uid"
#define BOX_ISMOUNT_KEY "isMount"

BoxManager::BoxManager(QObject *parent) : QObject(parent)
{
    this->m_dbusAdaptor = new BoxManagerAdaptor(this);

    this->init();
}
BoxManager::~BoxManager()
{
}

QString BoxManager::CreateBox(const QString &name, const QString &password)
{
    auto decryptPasswd = CryptoHelper::rsaDecrypt(this->m_rsaPrivateKey, password);
    Box *box = new Box(name, decryptPasswd, getSenderUid());

    m_boxs.insert(box->getBoxID(), box);

    //    auto decryptPassphrase = CryptoHelper::rsaEncrypt(this->m_rsaPublicKey, box->getPassphrase());
    emit BoxAdded(box->getBoxID(), box->getPassphrase());
    return box->getBoxID();
}

bool BoxManager::DelBox(const QString &box_uid, const QString &boxId)
{
    auto box = m_boxs.value(box_uid);
    auto decryptInputPassword = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, boxId);

    RETURN_VAL_IF_TRUE(!box->delBox(decryptInputPassword), false)
    m_boxs.remove(box_uid);
    emit BoxDeleted(box_uid);

    return true;
}

QString BoxManager::GetBoxByUID(const QString &boxId)
{
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;
    auto box = m_boxs.value(boxId);

    jsonObj = QJsonObject{
        {BOX_NAME_KEY, box->getBoxName()},
        {BOX_UID_KEY, boxId},
        {BOX_ISMOUNT_KEY, box->isMount()}};
    jsonDoc.setObject(jsonObj);

    return QString(jsonDoc.toJson());
}

QString BoxManager::GetBoxs()
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArr;

    for (auto it = m_boxs.begin(); it != m_boxs.end(); it++)
    {
        auto box = it.value();
        // 调用者uid检测，不属于调用者创建的box不返回给前台
        if (getSenderUid() != box->getUserUid())
        {
            continue;
        }
        // 暂只返回以下三个数据给前台
        QJsonObject jsonObj{
            {BOX_NAME_KEY, box->getBoxName()},
            {BOX_UID_KEY, box->getBoxID()},
            {BOX_ISMOUNT_KEY, box->isMount()}};

        jsonArr.push_back(jsonObj);
    }

    jsonDoc.setArray(jsonArr);
    return QString(jsonDoc.toJson());
}

bool BoxManager::IsMounted(const QString &boxId)
{
    auto box = m_boxs.value(boxId);

    return box->isMount();
}

bool BoxManager::ModifyBoxPassword(const QString &boxId,
                                   const QString &currentPassword,
                                   const QString &newPassword)
{
    auto decryptInputPassword = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, currentPassword);
    auto decryptNewPassword = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, newPassword);

    auto box = m_boxs.value(boxId);

    return box->modifyBoxPassword(decryptInputPassword, decryptNewPassword);
}

// 解锁
bool BoxManager::Mount(const QString &boxId, const QString &password)
{
    auto decryptInputPassword = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, password);
    auto box = m_boxs.value(boxId);

    if (box->getBoxID() == boxId)
    {
        emit BoxChanged(boxId);
        return box->mount(decryptInputPassword);
    }

    return false;
}

bool BoxManager::RetrievePassword(const QString &boxId, const QString &passphrase, const QString &newPassword)
{
    auto decryptPassphrase = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, passphrase);
    auto decryptNewPassword = CryptoHelper::rsaDecrypt(m_rsaPrivateKey, newPassword);

    auto box = m_boxs.value(boxId);
    return box->retrievePassword(decryptPassphrase, decryptNewPassword);
}

// 上锁
void BoxManager::UnMount(const QString &boxId)
{
    auto box = m_boxs.value(boxId);

    if (box->getBoxID() == boxId)
    {
        emit BoxChanged(boxId);
        return box->umount();
    }
}

void BoxManager::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(SC_BOX_MANAGER_DBUS_OBJECT_PATH, this))
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
        Box *box = new Box(boxInfo.boxName, decryptPassword, boxInfo.userUid, boxInfo.boxId);
        m_boxs.insert(boxInfo.boxId, box);
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
