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

#include "accounts-fake.h"

namespace KS
{
AccountsFake::AccountsFake(QObject *parent)
    : Accounts(parent)
{
}

AccountRole AccountsFake::getRole(const QString &dbusUniqueName) const
{
    return AccountRole::ACCOUNT_ROLE_NOACCOUNT;
}

AccountRole AccountsFake::getRole(pid_t dbusPid) const
{
    return AccountRole::ACCOUNT_ROLE_NOACCOUNT;
}

QString AccountsFake::getUserName(const QString &dbusUniqueName) const
{
    return QString();
}

QString AccountsFake::getUserName(pid_t dbusPid) const
{
    return QString();
}

}  // namespace KS
