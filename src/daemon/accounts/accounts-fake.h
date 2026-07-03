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

#include "accounts.h"

namespace KS
{
class AccountsFake : public Accounts
{
    Q_OBJECT

public:
    AccountsFake(QObject *parent = nullptr);
    virtual ~AccountsFake(){};

    virtual AccountRole getRole(const QString &dbusUniqueName) const;
    virtual AccountRole getRole(pid_t dbusPid) const;
    virtual QString getUserName(const QString &dbusUniqueName) const;
    virtual QString getUserName(pid_t dbusPid) const;
};

}  // namespace KS
