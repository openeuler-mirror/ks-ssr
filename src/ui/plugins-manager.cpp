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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "plugins-manager.h"
#include <qt5-log-i.h>
#include <ssr-i.h>
#include <ssr-marcos.h>
#include <ui-plugin-i.h>
#include <QDir>
#include <QGlobalStatic>
#include <QJsonArray>
#include <QJsonObject>
#include <QPluginLoader>
#include <QScopedPointer>
#include <QSettings>
#include "account_proxy.h"
#include "config.h"
#include "daemon_proxy.h"

namespace KS
{
#define SSR_UI_PLUGIN_CONF SSR_INSTALL_DATADIR "/ssr-ui-plugins.conf"
#define SSR_UI_PLUGIN_KEY_ENABLED "enabled"

struct PluginPageInfo
{
    QString pageUID;
    QString requireRoleName;
    QString requireDaemon;
};

struct PluginMetaData
{
    QString id;
    QVector<PluginPageInfo> pagesInfo;
};

struct PluginInfo
{
    QSharedPointer<QPluginLoader> loader;
    PluginMetaData metaData;
    IUIPlugin *plugin;
};

PluginsManager::PluginsManager(QObject *parent)
    : QObject(parent)
{
    m_settings = new QSettings(SSR_UI_PLUGIN_CONF, QSettings::IniFormat);

    m_daemonProxy = new DaemonProxy(SSR_DBUS_NAME,
                                    SSR_DBUS_OBJECT_PATH,
                                    QDBusConnection::systemBus(),
                                    this);

    m_accountProxy = new AccountProxy(SSR_DBUS_NAME,
                                      SSR_ACCOUNT_DBUS_OBJECT_PATH,
                                      QDBusConnection::systemBus(),
                                      this);
}

PluginsManager::~PluginsManager()
{
    // deactivatePlugins();
}

void PluginsManager::init()
{
    initPlugins();
}

// void PluginsManager::activatePlugins()
// {
//     QStringList activatedPlugins;

//     for (auto &pluginInfo : m_plugins)
//     {
//         pluginInfo->plugin->activate();
//         activatedPlugins.push_back(pluginInfo->loader->fileName());
//     }

//     KLOG_INFO() << "Activated plugins: " << activatedPlugins;
// }

// void PluginsManager::deactivatePlugins()
// {
//     QStringList deactivatedPlugins;

//     for (auto &pluginInfo : m_plugins)
//     {
//         pluginInfo->plugin->deactivate();
//         deactivatedPlugins.push_back(pluginInfo->loader->fileName());
//     }

//     KLOG_INFO() << "Deactivated plugins: " << deactivatedPlugins;
// }

QVector<Page *> PluginsManager::createAvailablePages()
{
    auto role = m_accountProxy->GetLoginRole().value();
    auto roleName = m_accountProxy->GetRoleName(role).value();
    auto availableDaemonPlugins = m_daemonProxy->GetAvailablePlugins().value();
    QStringList createdPages;

    QVector<Page *> pages;
    for (auto &pluginInfo : m_plugins)
    {
        for (auto &pageInfo : pluginInfo->metaData.pagesInfo)
        {
            // 判断登录角色是否有页面访问权限
            if (!pageInfo.requireRoleName.isEmpty() && pageInfo.requireRoleName != roleName)
            {
                KLOG_INFO() << "Ingore page" << pageInfo.pageUID << ", because of role name dismatch.";
                continue;
            }

            // 判断后端依赖插件是否加载，否则前端不应该显示
            if (!availableDaemonPlugins.contains(pageInfo.requireDaemon))
            {
                KLOG_INFO() << "Ingore page" << pageInfo.pageUID << ", because of required daemon module isn't loaded.";
                continue;
            }

            auto page = pluginInfo->plugin->createPage(pageInfo.pageUID);
            if (!page)
            {
                KLOG_WARNING() << "Failed to create page for" << pageInfo.pageUID;
                continue;
            }
            pages.push_back(page);
            createdPages.push_back(pageInfo.pageUID);
        }
    }

    KLOG_INFO() << "Created pages: " << createdPages;
    return pages;
}

void PluginsManager::initPlugins()
{
    QDir dir(SSR_INSTALL_UI_PLUGINDIR);

    QStringList loadedPlugins;

    for (auto &entryInfo : dir.entryInfoList())
    {
        CONTINUE_IF_TRUE(entryInfo.isDir());

        if (!entryInfo.fileName().endsWith(".so"))
        {
            KLOG_WARNING() << "Ignore file " << entryInfo.absoluteFilePath() << ", because it doesn't end with so.";
            continue;
        }

        auto pluginInfo = QSharedPointer<PluginInfo>(new PluginInfo());
        pluginInfo->loader = QSharedPointer<QPluginLoader>::create(entryInfo.absoluteFilePath());
        auto metaDataJson = pluginInfo->loader->metaData();
        auto iid = metaDataJson.value("IID").toString();
        if (iid != IUI_IID)
        {
            KLOG_WARNING() << "Incompatible plugin" << entryInfo.absoluteFilePath() << ".";
            continue;
        }

        auto object = metaDataJson.value("MetaData").toObject();
        pluginInfo->metaData.id = object.value("id").toString();

        auto pages = object.value("pages").toArray();
        for (auto page : pages)
        {
            auto pageObject = page.toObject();
            PluginPageInfo pageInfo;
            pageInfo.pageUID = pageObject.value("pageUID").toString();
            pageInfo.requireRoleName = pageObject.value("requireRoleName").toString();
            pageInfo.requireDaemon = pageObject.value("requireDaemon").toString();
            pluginInfo->metaData.pagesInfo.append(pageInfo);
        }

        pluginInfo->plugin = qobject_cast<IUIPlugin *>(pluginInfo->loader->instance());
        if (!pluginInfo->plugin)
        {
            KLOG_WARNING() << "Failed to create instance for plugin" << entryInfo.absoluteFilePath()
                           << ", reason is" << pluginInfo->loader->errorString();
            continue;
        }

        m_plugins.insert(pluginInfo->metaData.id, pluginInfo);
        loadedPlugins.push_back(pluginInfo->loader->fileName());
    }

    KLOG_INFO() << "Loaded plugins:" << loadedPlugins;
}

}  // namespace KS
