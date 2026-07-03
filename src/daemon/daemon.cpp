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

#include "daemon.h"
#include <qt5-log-i.h>
#include <ssr-i.h>
#include <ssr-marcos.h>
#include <QDBusConnection>
#include "accounts/accounts-entity.h"
#include "accounts/accounts-fake.h"
#include "authentication.h"
#include "config.h"
#include "daemon_adaptor.h"
#include "lib/dbus/license-proxy.h"
#include "log/log-manager.h"
#include "plugins-manager.h"

namespace KS
{
Daemon *Daemon::m_instance = nullptr;

// 需要给插件提供接口
IDaemonLog *g_logManager = nullptr;
IDaemonAuthentication *g_daemonAuthentication = nullptr;

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
      m_accounts(nullptr),
      m_started(false)
{
    m_dbusAdaptor = new DaemonAdaptor(this);
    m_licenseProxy = LicenseProxy::getDefault();
#ifdef ENABLE_ACCOUNTS_MANAGER
    m_accounts = new AccountsEntity(this);
#else
    m_accounts = new AccountsFake(this);
#endif
    g_logManager = m_log = new Log::Manager(m_accounts, this);
    m_pluginManager = new PluginsManager(this);
    g_daemonAuthentication = m_authentication = new Authentication(m_accounts, this);

    connect(m_licenseProxy.data(), &LicenseProxy::activated, this, &Daemon::start);
}

Daemon::~Daemon()
{
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
