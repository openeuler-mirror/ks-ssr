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

#pragma once

#include "user.h"

class AccountProxy;

namespace KS
{
class PasswordModification;

class Login;

class UserEntity : public User
{
    Q_OBJECT

public:
    UserEntity(QWidget *parent = nullptr);
    virtual ~UserEntity(){};

    virtual void init();
    // 显示修改密码界面
    virtual void showPasswordModification();
    // 显示登录界面
    virtual void showLogin();
    // 退出用户
    virtual bool logout();
    virtual QString getCurrentUserName() const;

private slots:
    void acceptedLogin();
    void acceptedPasswordModification();

private:
    static UserEntity *m_instance;

    AccountProxy *m_dbusProxy;
    Login *m_login;
    PasswordModification *m_passwordModification;
    QString m_currentUserName;
};

}  // namespace KS
