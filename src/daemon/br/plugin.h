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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#pragma once

#include <QDir>
#include <QFileInfo>
#include <QString>
#include "plugin-loader.h"
#include "reinforcement.h"

namespace KS
{
namespace BRDaemon
{
class Plugin
{
public:
    Plugin(const QString& conf_path);
    virtual ~Plugin();

    // 初始化
    bool init();

    // 获取插件ID，由插件名称+语言拼接
    QString getId()
    {
        return QString::fromStdString(this->plugin_config_->name() + "_" + this->plugin_config_->language_type());
    };
    // 获取插件名称
    std::string getName()
    {
        return this->plugin_config_->name();
    };
    // 插件所属分类
    std::string getCategoryName()
    {
        return this->plugin_config_->category();
    };
    // 获取插件加载器
    std::shared_ptr<PluginLoader> getLoader()
    {
        return this->loader_;
    };
    // 获取加固项名称列表
    std::vector<std::string> getReinforcementNames();
    // 获取加固项配置
    const Protocol::Reinforcement* getReinforcementConfig(const std::string& name);

private:
    // 使用插件加载器加载插件
    bool loadPluginModule();

private:
    // 插件配置文件路径
    QString conf_path_;
    // 插件配置
    std::unique_ptr<Protocol::Plugin> plugin_config_;
    // 插件状态
    // BRPluginState state_;
    // 插件加载器
    std::shared_ptr<PluginLoader> loader_;

    // 属于该插件的加固项信息：<加固项名称，加固项信息>
    // std::map<std::string, std::shared_ptr<BRReinforcement>> reinforcements_;
};

typedef QVector<QSharedPointer<Plugin>> BRPluginVec;
}  // namespace BRDaemon
}  // namespace KS