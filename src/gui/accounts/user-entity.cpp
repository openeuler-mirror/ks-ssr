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
 * Author:     chendingjian <chendingjian@kylinsec.com.cn>
 */

#include "user-entity.h"
#include <ssr-i.h>
#include <ssr-marcos.h>
#include "account_proxy.h"
#include "config.h"
#include "lib/base/crypto-helper.h"
#include "lib/widgets/password-modification.h"
#include "lib/widgets/ssr-marcos-ui.h"
#include "login.h"

#define USER_INFO_INITIAL_PASSWD "kylin.123"

namespace KS
{
UserEntity::UserEntity(QWidget *parent)
    : User(parent),
      m_dbusProxy(nullptr),
      m_login(nullptr),
      m_passwordModification(nullptr)
{
    m_dbusProxy = new AccountProxy(SSR_DBUS_NAME,
                                   SSR_ACCOUNT_DBUS_OBJECT_PATH,
                                   QDBusConnection::systemBus(),
                                   this);

    connect(m_dbusProxy, &AccountProxy::PasswordChanged, [this](const QString &userName)
            {
                if (userName == m_currentUserName)
                {
                    Q_EMIT passwordChanged();
                    logout();
                }
            });
}

void UserEntity::init()
{
    hide();
}

void UserEntity::showPasswordModification()
{
    // 修改密码界面
    if (!m_passwordModification)
    {
        m_passwordModification = new PasswordModification(this);
        // 不在关闭时销毁
        m_passwordModification->setAttribute(Qt::WA_DeleteOnClose, false);
        connect(m_passwordModification, &PasswordModification::accepted, this, &UserEntity::acceptedPasswordModification);
    }

    m_passwordModification->setTitleNameTail(m_currentUserName);
    auto x = window()->x() + window()->width() / 2 - m_passwordModification->width() / 2;
    auto y = window()->y() + window()->height() / 2 - m_passwordModification->height() / 2;
    m_passwordModification->move(x, y);
    m_passwordModification->clearLineText();
    m_passwordModification->show();
}

void UserEntity::showLogin()
{
    // 登录界面
    if (!m_login)
    {
        m_login = new Login(this);
        connect(m_login, &Login::accepted, this, &UserEntity::acceptedLogin);
        connect(m_login, &Login::rejected, this, &User::softExited);
    }

    auto x = window()->x() + window()->width() / 2 - m_login->width() / 2;
    auto y = window()->y() + window()->height() / 2 - m_login->height() / 2;
    m_login->move(x, y);
    m_login->raise();
    m_login->activateWindow();
    m_login->show();
}

bool UserEntity::logout()
{
    if (m_currentUserName.isEmpty())
    {
        POPUP_MESSAGE_DIALOG(tr("User is not login."));
        return false;
    }
    auto reply = m_dbusProxy->Logout();
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    RETURN_VAL_IF_TRUE(reply.isError(), false);
    m_currentUserName.clear();
    showLogin();
    return true;
}

QString UserEntity::getCurrentUserName() const
{
    return m_currentUserName;
}

void UserEntity::acceptedLogin()
{
    auto encryptPassword = CryptoHelper::rsaEncryptString(m_dbusProxy->rSAPublicKey(), m_login->getPassword());
    auto reply = m_dbusProxy->Login(m_login->getAccountName(), encryptPassword);
    reply.waitForFinished();
    // 这里不能使用POPUP_MESSAGE_DIALOG，主窗口还未显示，消息窗口为父类为window时，点击关闭会触发window的退出事件，
    // 使软件退出，这里登入失败了软件不应该退出
    if (reply.isError())
    {
        auto messageDialog = new KS::MessageDialog(m_login);
        messageDialog->setMessage(reply.error().message());
        int x = m_login->x() + m_login->width() / 2 - messageDialog->width() / 2;
        int y = m_login->y() + m_login->height() / 2 - messageDialog->height() / 2;
        messageDialog->move(x, y);
        messageDialog->show();
        return;
    }

    if (m_login->getPassword() == USER_INFO_INITIAL_PASSWD)
    {
        POPUP_MESSAGE_DIALOG(tr("Currently using the initial password to login, please change it as soon as possible!"));
    }
    m_currentUserName = m_login->getAccountName();
    emit loginFinished();
    m_login->setPassword("");
    m_login->hide();
}

void UserEntity::acceptedPasswordModification()
{
    auto encryptCurrentPassword = CryptoHelper::rsaEncryptString(m_dbusProxy->rSAPublicKey(), m_passwordModification->getCurrentPassword());
    auto encryptNewPassword = CryptoHelper::rsaEncryptString(m_dbusProxy->rSAPublicKey(), m_passwordModification->getNewPassword());
    auto reply = m_dbusProxy->ChangePassphrase(m_currentUserName, encryptCurrentPassword, encryptNewPassword);
    reply.waitForFinished();
    // 后台验证密码错误，此时前台已经将修改密码弹窗关闭了，需要进行再次显示（#25868）
    if (reply.isError())
    {
        showPasswordModification();
        POPUP_MESSAGE_DIALOG(reply.error().message());
        return;
    }
}

}  // namespace KS
