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

#include "manager.h"
#include "include/ssr-i.h"
#include "include/ssr-marcos.h"
#include "lib/base/crypto-helper.h"
#include "src/ui/account/login.h"
#include "src/ui/account_proxy.h"
#include "src/ui/common/password-modification.h"
#include "src/ui/common/ssr-marcos-ui.h"

#define USER_INFO_INITIAL_PASSWD "123123"

namespace KS
{
namespace Account
{
Manager *Manager::m_instance = nullptr;
void Manager::globalInit(QWidget *parent)
{
    if (!m_instance)
    {
        m_instance = new Manager(parent);
    }
}

void Manager::globalDeinit()
{
    if (m_instance)
    {
        delete m_instance;
        m_instance = nullptr;
    }
}

void Manager::showPasswordModification()
{
    // 修改密码界面
    m_passwordModification = new PasswordModification(this);
    connect(m_passwordModification, &PasswordModification::accepted, this, &Manager::acceptedPasswordModification);

    m_passwordModification->setTitleNameTail(m_currentUserName);
    auto x = window()->x() + window()->width() / 2 - m_passwordModification->width() / 2;
    auto y = window()->y() + window()->height() / 2 - m_passwordModification->height() / 2;
    m_passwordModification->move(x, y);
    m_passwordModification->show();
    m_passwordModification->show();
}

void Manager::showLogin()
{
    // 登录界面
    if (!m_login)
    {
        m_login = new Login(this);
        connect(m_login, &Login::accepted, this, &Manager::acceptedLogin);
        connect(m_login, &Login::rejected, this, &Manager::softExited);
    }
    auto x = window()->x() + window()->width() / 2 - m_login->width() / 2;
    auto y = window()->y() + window()->height() / 2 - m_login->height() / 2;
    m_login->move(x, y);
    m_login->show();
    m_login->show();
}

void Manager::logout()
{
    RETURN_IF_TRUE(m_currentUserName.isEmpty());
    auto reply = m_dbusProxy->Logout();
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    RETURN_IF_TRUE(reply.isError());
    m_currentUserName = "";
    showLogin();
}

void Manager::setLoginUserName(const QString &userName)
{
    m_login->setAccountName(userName);
}

QString Manager::getCurrentUserName() const
{
    return m_currentUserName;
}

Manager::Manager(QWidget *parent)
    : QWidget(parent),
      m_dbusProxy(nullptr),
      m_login(nullptr),
      m_passwordModification(nullptr)
{
    init();
}

void Manager::init()
{
    m_dbusProxy = new AccountProxy(SSR_DBUS_NAME,
                                   SSR_ACCOUNT_DBUS_OBJECT_PATH,
                                   QDBusConnection::systemBus(),
                                   this);
    connect(m_dbusProxy, &AccountProxy::PasswordChanged, this, &Manager::passwordChanged);
    hide();
}

void Manager::acceptedLogin()
{
    auto encryptPassword = CryptoHelper::rsaEncryptString(m_dbusProxy->rSAPublicKey(), m_login->getPassword());
    auto reply = m_dbusProxy->Login(m_login->getAccountName(), encryptPassword);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    RETURN_IF_TRUE(reply.isError());

    if (m_login->getPassword() == USER_INFO_INITIAL_PASSWD)
    {
        POPUP_MESSAGE_DIALOG(tr("Currently using the initial password to login, please change it as soon as possible!"));
    }
    m_currentUserName = m_login->getAccountName();
    emit loginFinished();
    m_login->setPassword("");
    m_login->hide();
}

void Manager::acceptedPasswordModification()
{
    auto encryptCurrentPassword = CryptoHelper::rsaEncryptString(m_dbusProxy->rSAPublicKey(), m_passwordModification->getCurrentPassword());
    auto encryptNewPassword = CryptoHelper::rsaEncryptString(m_dbusProxy->rSAPublicKey(), m_passwordModification->getNewPassword());
    auto reply = m_dbusProxy->ChangePassphrase(m_currentUserName, encryptCurrentPassword, encryptNewPassword);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    RETURN_IF_TRUE(reply.isError());
}
}  // namespace Account
}  // namespace KS
