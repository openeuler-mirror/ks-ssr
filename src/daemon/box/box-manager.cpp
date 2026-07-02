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
    auto decryptPasswd = CryptoHelper::rsa_decrypt(this->m_rsaPrivateKey, password);
    Box *box = new Box(name, decryptPasswd, getSenderUid());

    m_boxList << box;

    emit BoxAdded(box->getBoxID());
    return box->getBoxID();
}

bool BoxManager::DelBox(const QString &box_uid, const QString &password)
{
    for (Box *box : m_boxList)
    {
        if (box->getBoxID() == box_uid)
        {
            auto decryptInputPassword = CryptoHelper::rsa_decrypt(m_rsaPrivateKey, password);

            RETURN_VAL_IF_TRUE(!box->delBox(decryptInputPassword), false)
            emit BoxDeleted(box_uid);
        }
    }

    return true;
}

QString BoxManager::GetBoxByUID(const QString &box_uid)
{
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;

    for (Box *box : m_boxList)
    {
        if (box->getBoxID() == box_uid)
        {
            jsonObj = QJsonObject{
                {BOX_NAME_KEY, box->getBoxName()},
                {BOX_UID_KEY, box_uid},
                {BOX_ISMOUNT_KEY, box->isMount()}};
            jsonDoc.setObject(jsonObj);

            return QString(jsonDoc.toJson());
        }
    }

    return "";
}

QString BoxManager::GetBoxs()
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArr;

    int i = 0;
    for (Box *box : m_boxList)
    {
        // 调用者uid检测，不属于调用者创建的box不返回给前台
        if (getSenderUid() != box->getUserUid())
        {
            i++;
            continue;
        }
        // 暂只返回以下三个数据给前台
        QJsonObject jsonObj{
            {BOX_NAME_KEY, box->getBoxName()},
            {BOX_UID_KEY, box->getBoxID()},
            {BOX_ISMOUNT_KEY, box->isMount()}};

        jsonArr.push_back(jsonObj);
        i++;
    }

    jsonDoc.setArray(jsonArr);
    return QString(jsonDoc.toJson());
}

bool BoxManager::IsMounted(const QString &box_uid)
{
    for (Box *box : m_boxList)
    {
        if (box->getBoxID() == box_uid)
        {
            return box->isMount();
        }
    }
    return false;
}

bool BoxManager::ModifyBoxPassword(const QString &box_uid,
                                   const QString &current_password,
                                   const QString &new_password)
{
    auto decryptInputPassword = CryptoHelper::rsa_decrypt(m_rsaPrivateKey, current_password);
    auto decryptNewPassword = CryptoHelper::rsa_decrypt(m_rsaPrivateKey, new_password);

    for (Box *box : m_boxList)
    {
        if (box->getBoxID() == box_uid)
        {
            return box->modifyBoxPassword(decryptInputPassword, decryptNewPassword);
        }
    }

    return false;
}

// 解锁
bool BoxManager::Mount(const QString &box_uid, const QString &password)
{
    auto decryptInputPassword = CryptoHelper::rsa_decrypt(m_rsaPrivateKey, password);

    for (Box *box : m_boxList)
    {
        if (box->getBoxID() == box_uid)
        {
            emit BoxChanged(box_uid);
            return box->mount(decryptInputPassword);
        }
    }
    return false;
}

// 上锁
void BoxManager::UnMount(const QString &box_uid)
{
    for (Box *box : m_boxList)
    {
        if (box->getBoxID() == box_uid)
        {
            emit BoxChanged(box_uid);
            return box->umount();
        }
    }
}

void BoxManager::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerObject(SC_BOX_MANAGER_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }

    KS::CryptoHelper::generate_rsa_key(RSA_KEY_LENGTH, m_rsaPrivateKey, m_rsaPublicKey);

    m_boxList = {};
    BoxDao *boxDao = new BoxDao;
    auto boxInfoList = boxDao->getBoxs();
    for (BoxDaoInfo boxInfo : boxInfoList)
    {
        auto decryptPassword = CryptoHelper::aes_decrypt(boxInfo.encryptpassword);
        Box *box = new Box(boxInfo.boxName, decryptPassword, boxInfo.senderUserUid, boxInfo.boxId);
        m_boxList << box;
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
