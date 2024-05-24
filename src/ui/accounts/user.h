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

#pragma once

#include <QWidget>

class AccountProxy;

namespace KS
{
class PasswordModification;
namespace Accounts
{
class Login;

class User : public QWidget
{
    Q_OBJECT

public:
    User(QWidget *parent = nullptr);
    virtual ~User(){};

    void init();

    // 显示修改密码界面
    void showPasswordModification();
    // 显示登录界面
    void showLogin();
    // 退出用户
    bool logout();
    void setLoginUserName(const QString &userName);
    QString getCurrentUserName() const;

private slots:
    void acceptedLogin();
    void acceptedPasswordModification();

signals:
    void softExited();
    void loginFinished();
    void passwordChanged(const QString &userName);

private:
    static User *m_instance;

    AccountProxy *m_dbusProxy;
    Login *m_login;
    PasswordModification *m_passwordModification;
    QString m_currentUserName;
};

}  // namespace Accounts
}  // namespace KS
