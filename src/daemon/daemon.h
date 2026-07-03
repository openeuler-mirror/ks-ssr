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

#pragma once

#include <QDBusContext>
#include <QSharedPointer>

class DaemonAdaptor;

namespace KS
{
class LicenseProxy;
class PluginsManager;
class Accounts;
class Authentication;

namespace Log
{
class Manager;
}

class Daemon : public QObject,
               protected QDBusContext
{
    Q_OBJECT

public:
    static void globalInit();
    static void globalDeinit();
    static Daemon *getInstance();

    void start();

private:
    Daemon();
    virtual ~Daemon();

    void init();

public Q_SLOTS:  // METHODS
    QStringList GetAvailablePlugins();

private:
    static Daemon *m_instance;
    DaemonAdaptor *m_dbusAdaptor;
    QSharedPointer<LicenseProxy> m_licenseProxy;
    Accounts *m_accounts;
    Log::Manager *m_log;
    Authentication *m_authentication;
    PluginsManager *m_pluginManager;
    bool m_started;
};
}  // namespace KS
