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

#include <ssr-i.h>
#include <QObject>
#include <QString>

namespace KS
{
class IDaemonAccounts;
extern IDaemonAccounts *g_accountsManager;

class IDaemonAccounts
{
public:
    virtual ~IDaemonAccounts(){};

    virtual AccountRole getRole(const QString &dbusUniqueName) const = 0;
    virtual AccountRole getRole(pid_t dbusPid) const = 0;
    virtual QString getUserName(const QString &dbusUniqueName) const = 0;
    virtual QString getUserName(pid_t dbusPid) const = 0;
    virtual QString accountRoleEnum2Str(AccountRole role) const = 0;
    virtual AccountRole accountRoleStr2Enum(const QString &roleStr) const = 0;
};
}  // namespace KS
