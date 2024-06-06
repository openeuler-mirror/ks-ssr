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

#include "src/daemon/authentication.h"
#include <daemon-log-i.h>
#include <ssr-marcos.h>
#include <QDBusConnection>
#include "accounts/accounts-entity.h"
#include "lib/base/error.h"
#include "lib/dbus/polkit-proxy.h"

namespace KS
{
Authentication::Authentication(Accounts *accounts, QObject *parent)
    : QObject(parent),
      m_accounts(accounts)
{
}

void Authentication::checkAuthorization(const QString &action,
                                        const QVector<AccountRole> &validateRole,
                                        const QDBusMessage &message,
                                        checkAuthHandler handler)
{
    // 如果启用角色管理功能，则通过角色管理控制权限，否则通过polkit控制权限
    if (qobject_cast<AccountsEntity *>(m_accounts))
    {
        auto uniqueName = message.service();
        auto role = m_accounts->getRole(uniqueName);
        if (!validateRole.contains(role))
        {
            DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED, message);
        }
        else
        {
            handler(message);
        }
    }
    else
    {
        PolkitProxy::getDefault()->checkAuthorization(action, true, message, handler);
    }
}

}  // namespace KS
