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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#pragma once

#include "plugin.h"

class QTimer;

namespace KS
{
namespace BR
{
class Configuration;
class Categories;

class Plugins : public QObject
{
    Q_OBJECT
public:
    Plugins(Configuration* configuration, QObject* parent = nullptr);
    virtual ~Plugins();

    // 初始化
    void init();

    // 获取所有插件信息
    BRPluginVec getPlugins()
    {
        return MapHelper::getValues(this->m_plugins);
    };
    // 获取插件信息，如果不存在则返回空指针
    QSharedPointer<Plugin> getPlugin(const QString& id)
    {
        return MapHelper::getValue(this->m_plugins, id);
    };
    // 通过加固项名称获取插件
    QSharedPointer<Plugin> getPluginByReinforcement(const QString& name);

    // 获取使用的加固项
    QSharedPointer<Reinforcement> getReinforcement(const QString& name)
    {
        return MapHelper::getValue(this->m_reinforcements, name);
    }
    // 获取所有加固项名称
    QStringList getReinforcementNames()
    {
        return m_reinforcements.keys();
    };
    BRReinforcementVec getReinforcements()
    {
        return m_reinforcements.values();
    };
    // 获取指定分类的加固项
    BRReinforcementVec getReinforcementsByCategory(const QString& category_name);

    QSharedPointer<BRReinforcementInterface> getReinforcementInterface(const QString& pluginName,
                                                                       const QString& reinforcementName);

Q_SIGNALS:
    void reinforcementsChanged();

private:
    // 加载所有插件配置
    void loadPlugins();
    void loadPluginsFromDir(const QString& dirname);
    // 添加插件
    bool addPlugin(QSharedPointer<Plugin> plugin);
    // 加载与加固标准相关的加固项
    void idleLoadReinforcements();
    void loadReinforcements();
    // 合并加固项
    void joinReinforcement(Protocol::Reinforcement& destReinforcement,
                           const Protocol::Reinforcement& sourceReinforcement);

    QString noop2Translate(const QString& souceTxt);

private:
    static Plugins* m_instance;

    Configuration* m_configuration;

    // 所有插件信息：<插件ID，插件>
    QMap<QString, QSharedPointer<Plugin>> m_plugins;
    // 正在使用的加固项信息：<加固项名称，加固信息>
    QMap<QString, QSharedPointer<Reinforcement>> m_reinforcements;
    // <加固项名称，插件>
    QMap<QString, QWeakPointer<Plugin>> m_reinforcementsPlugins;
    //
    QTimer* m_loadReinforcementTimer;
};
}  // namespace BR
}  // namespace KS
