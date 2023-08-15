/**
 * @file          /kiran-sse-manager/lib/core/sse-plugins.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "lib/core/sse-plugin.h"

namespace Kiran
{
class SSEConfiguration;
class SSECategories;

class SSEPlugins
{
public:
    SSEPlugins(SSEConfiguration* configuration);
    virtual ~SSEPlugins();

    static SSEPlugins* get_instance() { return instance_; };

    static void global_init(SSEConfiguration* configuration);

    static void global_deinit() { delete instance_; };

    ThreadPool& get_thread_pool() { return this->thread_pool_; }

    // 获取所有插件信息
    SSEPluginVec get_plugins() { return MapHelper::get_values(this->plugins_); };
    // 获取插件信息，如果不存在则返回空指针
    std::shared_ptr<SSEPlugin> get_plugin(const std::string& name) { return MapHelper::get_value(this->plugins_, name); };

    // 获取使用的加固项
    std::shared_ptr<SSEReinforcement> get_used_reinforcement(const std::string& name);
    SSEReinforcementVec get_used_reinforcements();
    // 获取指定分类的加固项
    SSEReinforcementVec get_used_reinforcements_by_category(const std::string& category_name);
    // 设置加固项的自定义加固参数
    bool set_reinforcement_arguments(const std::string& name, const std::string& custom_args);

private:
    // 初始化
    void init();

    // 获取加固项
    std::shared_ptr<SSEReinforcement> get_reinforcement(const std::string& name);

    // 加载所有插件配置
    void load_plugins();
    // 添加插件
    bool add_plugin(std::shared_ptr<SSEPlugin> plugin);

    // 加载与加固标准相关的加固项
    void load_use_reinforcements();
    // 加载自定义加固参数
    void load_ra();
    // 写入自定义加固参数
    bool write_ra();

private:
    static SSEPlugins* instance_;

    SSEConfiguration* configuration_;

    // 线程池
    ThreadPool thread_pool_;

    // 所有插件信息：<插件名，插件信息>
    std::map<std::string, std::shared_ptr<SSEPlugin>> plugins_;

    // 正在使用的加固项
    std::map<std::string, std::weak_ptr<SSEReinforcement>> used_reinforcements_;

    // 所有加固项信息：<加固项名称，加固信息>
    std::map<std::string, std::weak_ptr<SSEReinforcement>> reinforcements_;
};
}  // namespace Kiran