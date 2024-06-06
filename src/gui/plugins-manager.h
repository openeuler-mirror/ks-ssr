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

#pragma once

#include <QMap>
#include <QSharedPointer>

class QPluginLoader;
class QSettings;
class DaemonProxy;
class AccountProxy;

namespace KS
{
class PluginInfo;
class WorkPage;
class SettingPage;

class PluginsManager : public QObject
{
    Q_OBJECT

public:
    PluginsManager(QObject *parent);
    virtual ~PluginsManager();

    void init();

    QVector<WorkPage *> createAvailableWorkPages();
    QVector<SettingPage *> createAvailableSettingPages();

private:
    void initPlugins();
    void deinitPlugins();

private:
    // 插件开关配置
    QSettings *m_settings;
    // <插件ID，插件对象>
    QMap<QString, QSharedPointer<PluginInfo>> m_plugins;
    // 后端DBus代理
    DaemonProxy *m_daemonProxy;
    AccountProxy *m_accountProxy;
};
}  // namespace KS
