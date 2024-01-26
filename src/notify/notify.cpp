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

#include "notify.h"
#include "include/ssr-i.h"
#include "lib/base/notification-wrapper.h"
#include "toolbox_dbus_proxy.h"

namespace KS
{
namespace Notify
{
Notify::Notify(QObject *parent)
    : QObject(parent)
{
    ::Notify::NotificationWrapper::globalInit(tr("Security reinforcement").toStdString());

    m_dbusProxy = new ToolBoxDbusProxy(SSR_DBUS_NAME,
                                       SSR_TOOL_BOX_DBUS_OBJECT_PATH,
                                       QDBusConnection::systemBus(),
                                       this);
    connect(m_dbusProxy, &ToolBoxDbusProxy::HazardDetected, this, [](uint type, const QString &alertMessage) {
        // TODO 区分类型弹窗？
        Q_UNUSED(type)
        ::Notify::NOTIFY_ERROR(alertMessage.toUtf8());
    }, Qt::UniqueConnection);
}

Notify::~Notify()
{
    ::Notify::NotificationWrapper::globalDeinit();
}

}  // namespace Notify
}  // namespace KS
