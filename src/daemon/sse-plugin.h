/**
 * @file          /kiran-sse-manager/src/daemon/sse-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "src/daemon/sse-plugin-loader.h"
#include "src/daemon/sse-reinforcement.h"

namespace Kiran
{
enum SSEPluginLanguageType
{
    // c++
    SSE_PLUGIN_LANGUAGE_TYPE_CPP,
    // python
    SSE_PLUGIN_LANGUAGE_TYPE_PYTHON
};

// enum SSEPluginState
// {
//     // 未初始化
//     SSE_PLUGIN_STATE_UNINIT,
//     // 插件已初始化
//     SSE_PLUGIN_STATE_INITED,
//     // 插件已激活
//     SSE_PLUGIN_STATE_ACTIVED,
//     // 插件不可用
//     SSE_PLUGIN_STATE_UNAVAILABLE
// };

struct SSEPluginInfo
{
    // 插件名称
    std::string name;
    // 插件使用的开发语言
    SSEPluginLanguageType language_type;
    // 插件的加固项名称
    std::vector<std::string> reinforcements_name;
};

class SSEPlugin
{
public:
    SSEPlugin(const std::string &conf_path);
    virtual ~SSEPlugin();

    // 初始化
    bool init();

    // 获取插件名称
    std::string get_name() { return this->plugin_info_.name; };
    // 获取插件状态
    // SSEPluginState get_state() { return this->state_; };
    // 获取插件加载器
    std::shared_ptr<SSEPluginLoader> get_loader() { return this->loader_; };

    // 获取插件数据
    const SSEPluginInfo &get_plugin_info() { return this->plugin_info_; };

    // 获取插件的所有加固项
    SSEReinforcementVec get_reinforcements() { return MapHelper::get_values(this->reinforcements_); };

private:
    // 使用插件加载器加载插件
    bool load_plugin_module();
    // 加载加固项配置
    bool load_reinforcement(const Glib::KeyFile &keyfile, const std::string &reinforcement_name);

    // 添加加固项
    bool add_reinforcement(std::shared_ptr<SSEReinforcement> reinforcement);

private:
    // 插件配置文件路径
    std::string conf_path_;
    // 插件数据
    SSEPluginInfo plugin_info_;
    // 插件状态
    // SSEPluginState state_;
    // 插件加载器
    std::shared_ptr<SSEPluginLoader> loader_;

    // 属于该插件的加固项信息：<加固项名称，加固项信息>
    std::map<std::string, std::shared_ptr<SSEReinforcement>> reinforcements_;
};

using SSEPluginVec = std::vector<std::shared_ptr<SSEPlugin>>;
}  // namespace Kiran