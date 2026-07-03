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

#include "daemon.h"
#include <qt5-log-i.h>
#include <ssr-marcos.h>
#include <QDBusConnection>
#include "accounts/accounts-manager.h"
#include "daemon_adaptor.h"
#include "include/ssr-i.h"
#include "lib/dbus/license-proxy.h"
#include "log/log-manager.h"
#include "plugins-manager.h"

namespace KS
{
Daemon *Daemon::m_instance = nullptr;

IDaemonLog *g_logManager = nullptr;
IDaemonAccounts *g_accountsManager = nullptr;

void Daemon::globalInit()
{
    m_instance = new Daemon();
    m_instance->init();
};

void Daemon::globalDeinit()
{
    delete m_instance;
};

Daemon *Daemon::getInstance()
{
    return m_instance;
};

Daemon::Daemon()
    : QObject(nullptr),
      m_started(false)
{
    m_dbusAdaptor = new DaemonAdaptor(this);
    m_licenseProxy = LicenseProxy::getDefault();
    m_pluginManager = new PluginsManager(this);
    g_accountsManager = new Accounts::Manager();
    g_logManager = new Log::Manager(g_accountsManager);

    connect(m_licenseProxy.data(), &LicenseProxy::activated, this, &Daemon::start);
}

Daemon::~Daemon()
{
    if (g_logManager)
    {
        delete g_logManager;
        g_logManager = nullptr;
    }

    if (g_accountsManager)
    {
        delete g_accountsManager;
        g_accountsManager = nullptr;
    }
}

void Daemon::init()
{
    m_pluginManager->init();

    // TODO: 这个应该要放到所有插件加载完毕后再调用
    // 注册后端服务DBUS名称
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

QStringList Daemon::GetAvailablePlugins()
{
    return m_pluginManager->getActivatedPluginIDs();
}

void Daemon::start()
{
    RETURN_IF_TRUE(m_started);
    RETURN_IF_FALSE(m_licenseProxy->isActivated());

    m_licenseProxy->disconnect(m_licenseProxy.data(), &LicenseProxy::activated, this, &Daemon::start);
    m_pluginManager->activatePlugins();

    m_started = true;
}
}  // namespace KS
