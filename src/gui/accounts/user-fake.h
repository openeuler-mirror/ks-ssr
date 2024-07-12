/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#pragma once

#include "user.h"

namespace KS
{
class UserFake : public User
{
    Q_OBJECT

public:
    UserFake(QWidget *parent = nullptr);
    virtual ~UserFake(){};

    virtual void init() override{};
    // 显示修改密码界面
    virtual void showPasswordModification() override{};
    // 显示登录界面
    virtual void showLogin() override;
    // 退出用户
    virtual bool logout() override
    {
        return true;
    };
    // 获取登录用户
    virtual QString getCurrentUserName() const override;
};

}  // namespace KS
