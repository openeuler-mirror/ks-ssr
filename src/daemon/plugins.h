/**
 * @file          /ks-ssr-manager/src/daemon/plugins.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "src/daemon/plugin.h"

namespace KS
{
namespace Daemon
{
class Configuration;
class Categories;

class Plugins
{
public:
    Plugins(Configuration* configuration);
    virtual ~Plugins();

    static Plugins* get_instance() { return instance_; };

    static void global_init(Configuration* configuration);

    static void global_deinit() { delete instance_; };

    ThreadPool& get_thread_pool() { return this->thread_pool_; }

    // 获取所有插件信息
    SSRPluginVec get_plugins() { return MapHelper::get_values(this->plugins_); };
    // 获取插件信息，如果不存在则返回空指针
    std::shared_ptr<Plugin> get_plugin(const std::string& id) { return MapHelper::get_value(this->plugins_, id); };
    // 通过加固项名称获取插件
    std::shared_ptr<Plugin> get_plugin_by_reinforcement(const std::string& name);

    // 获取使用的加固项
    std::shared_ptr<Reinforcement> get_reinforcement(const std::string& name) { return MapHelper::get_value(this->reinforcements_, name); }
    SSRReinforcementVec get_reinforcements() { return MapHelper::get_values(this->reinforcements_); };
    // 获取指定分类的加固项
    SSRReinforcementVec get_reinforcements_by_category(const std::string& category_name);

    std::shared_ptr<SSRReinforcementInterface> get_reinfocement_interface(const std::string& plugin_name,
                                                                          const std::string& reinforcement_name);

private:
    // 初始化
    void init();
    // 加载所有插件配置
    void load_plugins();
    void load_plugins_from_dir(const std::string& dirname);
    // 添加插件
    bool add_plugin(std::shared_ptr<Plugin> plugin);
    // 加载与加固标准相关的加固项
    void load_reinforcements();

    void on_rs_changed_cb();

private:
    static Plugins* instance_;

    Configuration* configuration_;

    // 线程池
    ThreadPool thread_pool_;

    // 所有插件信息：<插件ID，插件>
    std::map<std::string, std::shared_ptr<Plugin>> plugins_;
    // 正在使用的加固项信息：<加固项名称，加固信息>
    std::map<std::string, std::shared_ptr<Reinforcement>> reinforcements_;
    // <加固项名称，插件>
    std::map<std::string, std::weak_ptr<Plugin>> reinforcements_plugins_;
};
}  // namespace Daemon
}  // namespace KS