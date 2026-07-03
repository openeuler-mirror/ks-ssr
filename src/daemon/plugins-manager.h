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

namespace KS
{
class PluginInfo;

class PluginsManager : public QObject
{
    Q_OBJECT

public:
    PluginsManager(QObject *parent);
    virtual ~PluginsManager();

    void init();
    // 激活插件
    void activatePlugins();
    // 取消激活插件
    void deactivatePlugins();
    // 获取已激活插件ID
    QStringList getActivatedPluginIDs();

private:
    void initPlugins();
    void deinitPlugins();

private:
    // 插件开关配置
    QSettings *m_settings;
    // <插件ID，插件对象>
    QMap<QString, QSharedPointer<PluginInfo>> m_plugins;
};
}  // namespace KS
