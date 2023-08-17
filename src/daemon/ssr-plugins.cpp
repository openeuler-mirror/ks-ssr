/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-plugins.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/ssr-plugins.h"
#include "src/daemon/ssr-configuration.h"
#include "ssr-config.h"

namespace Kiran
{
SSRPlugins::SSRPlugins(SSRConfiguration* configuration) : configuration_(configuration),
                                                          thread_pool_(this->configuration_->get_max_thread_num())
{
}

SSRPlugins::~SSRPlugins()
{
}

SSRPlugins* SSRPlugins::instance_ = nullptr;
void SSRPlugins::global_init(SSRConfiguration* configuration)
{
    instance_ = new SSRPlugins(configuration);
    instance_->init();
}

std::shared_ptr<SSRPlugin> SSRPlugins::get_plugin_by_reinforcement(const std::string& name)
{
    auto iter = this->reinforcements_plugins_.find(name);
    RETURN_VAL_IF_TRUE(iter == this->reinforcements_plugins_.end(), nullptr);
    return iter->second.lock();
}

SSRReinforcementVec SSRPlugins::get_reinforcements_by_category(const std::string& category_name)
{
    SSRReinforcementVec result;
    for (auto iter : this->reinforcements_)
    {
        if (iter.second->get_category_name() == category_name)
        {
            result.push_back(iter.second);
        }
    }
    return result;
}

std::shared_ptr<SSRReinforcementInterface> SSRPlugins::get_reinfocement_interface(const std::string& plugin_name,
                                                                                  const std::string& reinforcement_name)
{
    auto plugin = this->get_plugin(plugin_name);
    if (!plugin)
    {
        KLOG_WARNING("Plugin '%s' of the reinforcement '%s' is not found.",
                     plugin_name.c_str(),
                     reinforcement_name.c_str());
        return nullptr;
    }

    auto plugin_interface = plugin->get_loader()->get_interface();
    if (!plugin_interface)
    {
        KLOG_WARNING("The Plugin interface for %s is nullptr.", plugin_name.c_str());
        return nullptr;
    }

    auto reinforcement_interface = plugin_interface->get_reinforcement(reinforcement_name);
    if (!reinforcement_interface)
    {
        KLOG_WARNING("The reinforcement interface for %s is nullptr.", reinforcement_name.c_str());
        return nullptr;
    }
    return reinforcement_interface;
}

void SSRPlugins::init()
{
    this->load_plugins();
    this->load_reinforcements();

    this->configuration_->signal_rs_changed().connect(sigc::mem_fun(this, &SSRPlugins::on_rs_changed_cb));
}

void SSRPlugins::load_plugins()
{
    KLOG_PROFILE("");

    try
    {
        Glib::Dir plugin_dir(SSR_PLUGIN_ROOT_DIR);
        for (auto iter = plugin_dir.begin(); iter != plugin_dir.end(); ++iter)
        {
            auto basename = *iter;
            auto filename = Glib::build_filename(SSR_PLUGIN_ROOT_DIR, basename);

            if (!Glib::file_test(filename, Glib::FILE_TEST_IS_REGULAR) ||
                !Glib::str_has_prefix(basename, "ssr-plugin"))
            {
                KLOG_DEBUG("Skip file %s.", filename.c_str());
                continue;
            }
            auto plugin = std::make_shared<SSRPlugin>(filename);

            // 初始化->激活->添加插件
            if (plugin->init())
            {
                auto plugin_loader = plugin->get_loader();

                if (!plugin_loader->activate() || !this->add_plugin(plugin))
                {
                    plugin_loader->deactivate();
                }
            }
        }
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
    }
}

bool SSRPlugins::add_plugin(std::shared_ptr<SSRPlugin> plugin)
{
    RETURN_VAL_IF_FALSE(plugin, false);

    KLOG_DEBUG("plugin name: %s.", plugin->get_name().c_str());

    auto iter = this->plugins_.emplace(plugin->get_name(), plugin);
    if (!iter.second)
    {
        KLOG_WARNING("The plugin is already exist. name: %s.", plugin->get_name().c_str());
        return false;
    }

    auto reinforcement_names = plugin->get_reinforcement_names();
    for (auto reinforcement_name : reinforcement_names)
    {
        auto old_plugin = this->get_plugin_by_reinforcement(reinforcement_name);
        if (old_plugin)
        {
            KLOG_WARNING("The reinforcement %s is conflicted with other plugin. old plugin: %s, cur plugin: %s.",
                         reinforcement_name.c_str(),
                         old_plugin->get_name().c_str(),
                         plugin->get_name().c_str());
        }
        else
        {
            this->reinforcements_plugins_[reinforcement_name] = plugin;
        }
    }
    return true;
}

void SSRPlugins::load_reinforcements()
{
    KLOG_PROFILE("");

    this->reinforcements_.clear();

    auto rs = this->configuration_->get_rs();
    RETURN_IF_FALSE(rs);

    try
    {
        for (auto reinforcement_arg : rs->body().reinforcement())
        {
            auto reinforcement_name = reinforcement_arg.name();
            auto plugin = this->get_plugin_by_reinforcement(reinforcement_name);

            // 加固标准中的加固项如果没有插件支持，则加载失败
            if (!plugin)
            {
                KLOG_WARNING("The reinforcement %s is unsupported by any plugin.", reinforcement_name.c_str());
                this->reinforcements_.clear();
                return;
            }

            auto reinforcement_noarg = plugin->get_reinforcement_config(reinforcement_name);
            if (!reinforcement_noarg)
            {
                KLOG_WARNING("The config of reinforcement %s is empty.", reinforcement_name.c_str());
                this->reinforcements_.clear();
                return;
            }

            // 添加加固项的基本信息（分类和标签）
            if (reinforcement_noarg->category().present())
            {
                reinforcement_arg.category(reinforcement_noarg->category().get());
            }
            reinforcement_arg.label(reinforcement_noarg->label());

            auto reinforcement = std::make_shared<SSRReinforcement>(plugin->get_name(),
                                                                    reinforcement_arg);

            this->reinforcements_.emplace(reinforcement_name, reinforcement);
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        this->reinforcements_.clear();
    }
}

void SSRPlugins::on_rs_changed_cb()
{
    this->load_reinforcements();
}

}  // namespace Kiran