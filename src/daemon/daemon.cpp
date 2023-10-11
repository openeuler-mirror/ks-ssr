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
#include "src/daemon/dm/device-manager.h"
#include "src/daemon/kss/dbus.h"
#include "src/daemon/log/manager.h"

namespace KS
{
Daemon *Daemon::m_instance = nullptr;

Daemon::Daemon() : QObject(nullptr)
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
    KSS::DBus::globalDeinit();
    DM::DeviceManager::globalDeinit();
    Box::BoxManager::globalDeinit();
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
    Box::BoxManager::globalInit(this);
    DM::DeviceManager::globalInit(this);
    KSS::DBus::globalInit(this);
    BRDaemon::Configuration::globalInit(SSR_BR_INSTALL_DATADIR "/ssr.ini");
    BRDaemon::Categories::globalInit();
    BRDaemon::Plugins::globalInit(BRDaemon::Configuration::getInstance());
    BRDaemon::BRDBus::globalInit(nullptr);
    Log::Manager::globalInit();
}
}  // namespace KS
