/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "src/daemon/ssr-plugin-loader.h"
#include "src/daemon/ssr-reinforcement.h"

namespace Kiran
{
// enum SSRPluginState
// {
//     // 未初始化
//     SSR_PLUGIN_STATE_UNINIT,
//     // 插件已初始化
//     SSR_PLUGIN_STATE_INITED,
//     // 插件已激活
//     SSR_PLUGIN_STATE_ACTIVED,
//     // 插件不可用
//     SSR_PLUGIN_STATE_UNAVAILABLE
// };

class SSRPlugin
{
public:
    SSRPlugin(const std::string& conf_path);
    virtual ~SSRPlugin();

    // 初始化
    bool init();

    // 获取插件名称
    std::string get_name() { return this->plugin_config_->name(); };
    // 插件所属分类
    std::string get_category_name() { return this->plugin_config_->category(); };
    // 获取插件状态
    // SSRPluginState get_state() { return this->state_; };
    // 获取插件加载器
    std::shared_ptr<SSRPluginLoader> get_loader() { return this->loader_; };
    // 获取加固项名称列表
    std::vector<std::string> get_reinforcement_names();
    // 获取加固项配置
    const Protocol::Reinforcement* get_reinforcement_config(const std::string& name);

private:
    // 使用插件加载器加载插件
    bool load_plugin_module();

private:
    // 插件配置文件路径
    std::string conf_path_;
    // 插件配置
    std::unique_ptr<Protocol::Plugin> plugin_config_;
    // 插件状态
    // SSRPluginState state_;
    // 插件加载器
    std::shared_ptr<SSRPluginLoader> loader_;

    // 属于该插件的加固项信息：<加固项名称，加固项信息>
    // std::map<std::string, std::shared_ptr<SSRReinforcement>> reinforcements_;
};

using SSRPluginVec = std::vector<std::shared_ptr<SSRPlugin>>;
}  // namespace Kiran