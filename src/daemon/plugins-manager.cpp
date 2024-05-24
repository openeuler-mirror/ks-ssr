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
#include <daemon-plugin-i.h>
#include <qt5-log-i.h>
#include <ssr-marcos.h>
#include <QDir>
#include <QGlobalStatic>
#include <QJsonArray>
#include <QJsonObject>
#include <QPluginLoader>
#include <QScopedPointer>
#include <QSettings>
#include "config.h"

namespace KS
{
#define SSR_DAEMON_PLUGIN_CONF SSR_INSTALL_DATADIR "/ssr-daemon-plugins.conf"
#define SSR_DAEMON_PLUGIN_KEY_ENABLED "enabled"

struct PluginMetaData
{
    QString id;
};

struct PluginInfo
{
    QSharedPointer<QPluginLoader> loader;
    PluginMetaData metaData;
    IDaemonPlugin *plugin;
    bool activated = false;
};

PluginsManager::PluginsManager(QObject *parent)
    : QObject(parent)
{
    m_settings = new QSettings(SSR_DAEMON_PLUGIN_CONF, QSettings::IniFormat);
}

PluginsManager::~PluginsManager()
{
    deactivatePlugins();
}

void PluginsManager::init()
{
    initPlugins();
}

void PluginsManager::activatePlugins()
{
    QStringList activatedPlugins;

    for (auto &pluginInfo : m_plugins)
    {
        if (pluginInfo->activated)
        {
            KLOG_WARNING() << "Plugin " << pluginInfo->metaData.id << "is activated already.";
            continue;
        }
        pluginInfo->plugin->activate();
        pluginInfo->activated = true;
        activatedPlugins.push_back(pluginInfo->loader->fileName());
    }

    KLOG_INFO() << "Activated plugins: " << activatedPlugins;
}

void PluginsManager::deactivatePlugins()
{
    QStringList deactivatedPlugins;

    for (auto &pluginInfo : m_plugins)
    {
        if (!pluginInfo->activated)
        {
            KLOG_WARNING() << "Plugin " << pluginInfo->metaData.id << "is deactivated before.";
            continue;
        }
        pluginInfo->plugin->deactivate();
        pluginInfo->activated = false;
        deactivatedPlugins.push_back(pluginInfo->loader->fileName());
    }

    KLOG_INFO() << "Deactivated plugins: " << deactivatedPlugins;
}

QStringList PluginsManager::getActivatedPluginIDs()
{
    QStringList activatedPlugins;
    for (auto &pluginInfo : m_plugins)
    {
        if (pluginInfo->activated)
        {
            activatedPlugins.push_back(pluginInfo->metaData.id);
        }
    }
    return activatedPlugins;
}

void PluginsManager::initPlugins()
{
    QDir dir(SSR_INSTALL_DAEMON_PLUGINDIR);

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
        if (iid != IDAEMON_IID)
        {
            KLOG_WARNING() << "Incompatible plugin" << entryInfo.absoluteFilePath() << ".";
            continue;
        }

        auto object = metaDataJson.value("MetaData").toObject();
        pluginInfo->metaData.id = object.value("id").toString();

        pluginInfo->plugin = qobject_cast<IDaemonPlugin *>(pluginInfo->loader->instance());
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
