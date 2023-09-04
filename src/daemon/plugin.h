/**
 * @file          /ks-ssr-manager/src/daemon/plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "src/daemon/plugin-loader.h"
#include "src/daemon/reinforcement.h"

namespace KS
{
namespace Daemon
{
class Plugin
{
public:
    Plugin(const std::string& conf_path);
    virtual ~Plugin();

    // 初始化
    bool init();

    // 获取插件ID，由插件名称+语言拼接
    std::string get_id() { return this->plugin_config_->name() + "_" + this->plugin_config_->language_type(); };
    // 获取插件名称
    std::string get_name() { return this->plugin_config_->name(); };
    // 插件所属分类
    std::string get_category_name() { return this->plugin_config_->category(); };
    // 获取插件加载器
    std::shared_ptr<PluginLoader> get_loader() { return this->loader_; };
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
    std::shared_ptr<PluginLoader> loader_;

    // 属于该插件的加固项信息：<加固项名称，加固项信息>
    // std::map<std::string, std::shared_ptr<SSRReinforcement>> reinforcements_;
};

typedef std::vector<std::shared_ptr<Plugin>> SSRPluginVec;
}  // namespace Daemon
}  // namespace KS