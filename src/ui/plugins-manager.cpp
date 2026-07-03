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

struct PluginWorkPageInfo
{
    QString pageUID;
    QString requireRoleName;
    QString requireDaemon;
};

struct PluginSettingPageInfo
{
    QString pageUID;
    QString requireRoleName;
    QString requireDaemon;
};

struct PluginMetaData
{
    QString id;
    QVector<PluginWorkPageInfo> workPagesInfo;
    QVector<PluginSettingPageInfo> settingPagesInfo;
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
}

void PluginsManager::init()
{
    initPlugins();
}

QVector<WorkPage *> PluginsManager::createAvailableWorkPages()
{
    auto role = m_accountProxy->GetLoginRole().value();
    auto roleName = m_accountProxy->GetRoleName(role).value();
    auto availableDaemonPlugins = m_daemonProxy->GetAvailablePlugins().value();
    QStringList createdWorkPagesUID;
    QVector<WorkPage *> createdWorkPages;

    for (auto &pluginInfo : m_plugins)
    {
        for (auto &workPageInfo : pluginInfo->metaData.workPagesInfo)
        {
#ifdef ENABLE_ACCOUNTS_MANAGER
            // 判断登录角色是否有页面访问权限
            if (!workPageInfo.requireRoleName.isEmpty() &&
                workPageInfo.requireRoleName != roleName)
            {
                KLOG_INFO() << "Ingore work page" << workPageInfo.pageUID << ", because of role name dismatch.";
                continue;
            }
#endif

            // 判断后端依赖插件是否加载，否则前端不应该显示
            if (!workPageInfo.requireDaemon.isEmpty() &&
                !availableDaemonPlugins.contains(workPageInfo.requireDaemon))
            {
                KLOG_INFO() << "Ingore work page" << workPageInfo.pageUID << ", because of required daemon module isn't loaded.";
                continue;
            }

            auto workPage = pluginInfo->plugin->createWorkPage(workPageInfo.pageUID);
            if (!workPage)
            {
                KLOG_WARNING() << "Failed to create work page for" << workPageInfo.pageUID;
                continue;
            }
            createdWorkPages.push_back(workPage);
            createdWorkPagesUID.push_back(workPageInfo.pageUID);
        }
    }

    KLOG_INFO() << "Created work pages: " << createdWorkPagesUID;
    return createdWorkPages;
}

QVector<SettingPage *> PluginsManager::createAvailableSettingPages()
{
    auto role = m_accountProxy->GetLoginRole().value();
    auto roleName = m_accountProxy->GetRoleName(role).value();
    auto availableDaemonPlugins = m_daemonProxy->GetAvailablePlugins().value();
    QStringList createdSettingPagesUID;
    QVector<SettingPage *> createdSettingPages;

    for (auto &pluginInfo : m_plugins)
    {
        for (auto &settingPageInfo : pluginInfo->metaData.settingPagesInfo)
        {
            // 判断登录角色是否有页面访问权限
            if (!settingPageInfo.requireRoleName.isEmpty() &&
                settingPageInfo.requireRoleName != roleName)
            {
                KLOG_INFO() << "Ingore setting page" << settingPageInfo.pageUID << ", because of role name dismatch.";
                continue;
            }

            // 判断后端依赖插件是否加载，否则前端不应该显示
            if (!settingPageInfo.requireDaemon.isEmpty() &&
                !availableDaemonPlugins.contains(settingPageInfo.requireDaemon))
            {
                KLOG_INFO() << "Ingore setting page" << settingPageInfo.pageUID << ", because of required daemon module isn't loaded.";
                continue;
            }

            auto settingPage = pluginInfo->plugin->createSettingPage(settingPageInfo.pageUID);
            if (!settingPage)
            {
                KLOG_WARNING() << "Failed to create setting page for" << settingPageInfo.pageUID;
                continue;
            }
            createdSettingPages.push_back(settingPage);
            createdSettingPagesUID.push_back(settingPageInfo.pageUID);
        }
    }

    KLOG_INFO() << "Created setting pages: " << createdSettingPagesUID;
    return createdSettingPages;
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

        auto workPages = object.value("workPages").toArray();
        for (auto workPage : workPages)
        {
            auto pageObject = workPage.toObject();
            PluginWorkPageInfo workPageInfo;
            workPageInfo.pageUID = pageObject.value("pageUID").toString();
            workPageInfo.requireRoleName = pageObject.value("requireRoleName").toString();
            workPageInfo.requireDaemon = pageObject.value("requireDaemon").toString();
            pluginInfo->metaData.workPagesInfo.append(workPageInfo);
        }

        auto settingPages = object.value("settingPages").toArray();
        for (auto settingPage : settingPages)
        {
            auto pageObject = settingPage.toObject();
            PluginSettingPageInfo settingPageInfo;
            settingPageInfo.pageUID = pageObject.value("pageUID").toString();
            settingPageInfo.requireRoleName = pageObject.value("requireRoleName").toString();
            settingPageInfo.requireDaemon = pageObject.value("requireDaemon").toString();
            pluginInfo->metaData.settingPagesInfo.append(settingPageInfo);
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
