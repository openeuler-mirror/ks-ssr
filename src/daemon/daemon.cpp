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

#include "src/daemon/daemon.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include "include/ssr-i.h"
#include "lib/license/license-proxy.h"
#include "src/daemon/box/box-manager.h"
#include "src/daemon/device/device-manager.h"
#include "src/daemon/kss/kss-dbus.h"

namespace KS
{
Daemon *Daemon::m_instance = nullptr;

Daemon::Daemon() : QObject(nullptr), m_kssDBus(nullptr)
{
    m_licenseProxy = LicenseProxy::getDefault();
    if (m_licenseProxy->isActivated())
    {
        start();
    }
    else
    {
        connect(m_licenseProxy.data(), &LicenseProxy::licenseChanged, this, &Daemon::start);
    }
}

Daemon::~Daemon()
{
    if (m_kssDBus)
    {
        delete m_kssDBus;
    }

    DeviceManager::globalDeinit();
    BoxManager::globalDeinit();
}

void Daemon::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.registerService(SSR_DBUS_NAME))
    {
        KLOG_WARNING() << "Failed to register dbus name: " << SSR_DBUS_NAME;
    }

    if (!connection.registerObject(SSR_DBUS_OBJECT_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << connection.lastError();
    }
}

void Daemon::start()
{
    m_licenseProxy->disconnect(m_licenseProxy.data(), &LicenseProxy::licenseChanged, this, &Daemon::start);
    BoxManager::globalInit(this);
    DeviceManager::globalInit(this);
    // FIXME: 最好需要提供一个模块入口类，可以时KSSContext或者KSSManager，来管理dbus和wrapper
    m_kssDBus = new KSSDbus(this);
}
}  // namespace KS
