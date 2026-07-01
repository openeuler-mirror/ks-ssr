/**
 * @file          /kiran-sse-manager/lib/core/sse-plugins.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/core/sse-plugins.h"
#include "lib/core/sse-configuration.h"
#include "sse-config.h"

namespace Kiran
{
SSEPlugins::SSEPlugins(SSEConfiguration* configuration) : configuration_(configuration),
                                                          thread_pool_(this->configuration_->get_max_thread_num())
{
}

SSEPlugins::~SSEPlugins()
{
}

SSEPlugins* SSEPlugins::instance_ = nullptr;
void SSEPlugins::global_init(SSEConfiguration* configuration)
{
    instance_ = new SSEPlugins(configuration);
    instance_->init();
}

std::shared_ptr<SSEReinforcement> SSEPlugins::get_used_reinforcement(const std::string& name)
{
    auto iter = this->used_reinforcements_.find(name);
    RETURN_VAL_IF_TRUE(iter == this->used_reinforcements_.end(), nullptr);
    return iter->second.lock();
}

SSEReinforcementVec SSEPlugins::get_used_reinforcements()
{
    SSEReinforcementVec result;
    for (auto iter : this->used_reinforcements_)
    {
        auto reinforcement = iter.second.lock();
        if (reinforcement)
        {
            result.push_back(reinforcement);
        }
    }
    return result;
}

SSEReinforcementVec SSEPlugins::get_used_reinforcements_by_category(const std::string& category_name)
{
    SSEReinforcementVec result;
    for (auto iter : this->used_reinforcements_)
    {
        auto reinforcement = iter.second.lock();
        if (reinforcement && reinforcement->category_name == category_name)
        {
            result.push_back(reinforcement);
        }
    }
    return result;
}

bool SSEPlugins::set_reinforcement_arguments(const std::string& name, const std::string& custom_args)
{
    auto reinforcement = this->get_used_reinforcement(name);
    RETURN_VAL_IF_FALSE(reinforcement, false);
    reinforcement->custom_args = custom_args;
    return this->write_ra();
}

std::shared_ptr<SSEReinforcement> SSEPlugins::get_reinforcement(const std::string& name)
{
    auto iter = this->reinforcements_.find(name);
    RETURN_VAL_IF_TRUE(iter == this->reinforcements_.end(), nullptr);
    return iter->second.lock();
}

void SSEPlugins::init()
{
    this->load_plugins();
    this->load_ra();
    this->load_use_reinforcements();
}

void SSEPlugins::load_plugins()
{
    SETTINGS_PROFILE("");

    try
    {
        Glib::Dir plugin_dir(SSE_PLUGIN_ROOT_DIR);
        for (auto iter = plugin_dir.begin(); iter != plugin_dir.end(); ++iter)
        {
            auto basename = *iter;
            auto filename = Glib::build_filename(SSE_PLUGIN_ROOT_DIR, basename);

            if (!Glib::file_test(filename, Glib::FILE_TEST_IS_REGULAR) ||
                !Glib::str_has_prefix(basename, "sse-plugin"))
            {
                LOG_DEBUG("Skip file %s.", filename.c_str());
                continue;
            }
            auto plugin = std::make_shared<SSEPlugin>(filename);

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
        LOG_WARNING("%s.", e.what().c_str());
    }
}

bool SSEPlugins::add_plugin(std::shared_ptr<SSEPlugin> plugin)
{
    RETURN_VAL_IF_FALSE(plugin, false);

    LOG_DEBUG("plugin name: %s.", plugin->get_name().c_str());

    auto iter = this->plugins_.emplace(plugin->get_name(), plugin);
    if (!iter.second)
    {
        LOG_WARNING("The plugin is already exist. name: %s.", plugin->get_name().c_str());
        return false;
    }

    auto reinforcements = plugin->get_reinforcements();
    for (auto reinforcement : reinforcements)
    {
        // LOG_DEBUG("reinforcement name: %s.", reinforcement->name.c_str());

        auto old_reinforcement = this->get_reinforcement(reinforcement->name);
        if (old_reinforcement)
        {
            LOG_WARNING("The reinforcement %s is conflicted with other plugin. reinforcement name: %s, old plugin: %s, cur plugin: %s.",
                        old_reinforcement->name.c_str(),
                        old_reinforcement->plugin_name.c_str(),
                        reinforcement->plugin_name.c_str());
        }
        else
        {
            this->reinforcements_[reinforcement->name] = reinforcement;
        }
    }
    return true;
}

void SSEPlugins::load_use_reinforcements()
{
    SETTINGS_PROFILE("");

    auto rs_str = this->configuration_->get_rs();
    auto rs = StrUtils::str2json(rs_str);

    this->used_reinforcements_.clear();

    try
    {
        const auto& rs_items = rs["body"]["items"];
        for (uint32_t i = 0; i < rs_items.size(); ++i)
        {
            const auto& rs_item = rs_items[i];
            auto name = rs_item["name"].asString();

            // 加固标准中的加固项如果没有插件支持，则加载失败
            auto iter = this->reinforcements_.find(name);
            if (iter == this->reinforcements_.end() || !iter->second.lock())
            {
                LOG_WARNING("The reinforcement %s is unsupported by any plugin.", name.c_str());
                this->used_reinforcements_.clear();
                return;
            }
            auto reinforcement = iter->second.lock();
            reinforcement->rules = rs_item["rules"];
            reinforcement->default_args = rs_item["default_args"];
            reinforcement->layout = rs_item["layout"];

            this->used_reinforcements_.emplace(name, reinforcement);
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARNING("%s.", e.what());
        this->used_reinforcements_.clear();
    }
}

void SSEPlugins::load_ra()
{
    // 加载自定义加固参数
    do
    {
        try
        {
            auto ra_str = this->configuration_->get_custom_ra();
            auto ra = StrUtils::str2json(ra_str);

            if (ra.isNull())
            {
                break;
            }

            const auto& ra_items = ra["body"]["items"];
            for (uint32_t i = 0; i < ra_items.size(); ++i)
            {
                const auto& ra_item = ra_items[i];
                auto name = ra_item["name"].asString();

                // 没有对应的插件则忽略
                auto iter = this->reinforcements_.find(name);
                if (iter == this->reinforcements_.end() || !iter->second.lock())
                {
                    LOG_WARNING("The reinforcement %s isn't found.", name.c_str());
                    continue;
                }
                auto reinforcement = iter->second.lock();
                reinforcement->custom_args = ra_item["custom_args"];
            }
        }
        catch (const std::exception& e)
        {
            LOG_WARNING("%s.", e.what());
        }
    } while (0);
}

bool SSEPlugins::write_ra()
{
    try
    {
        Json::Value ra_values;
        ra_values["head"]["version"] = PROJECT_VERSION;
        int32_t item_count = 0;

        for (auto iter : this->reinforcements_)
        {
            auto reinforcement = iter.second.lock();
            if (!reinforcement)
            {
                LOG_WARNING("The reinforcement %s isn't found.", iter.first.c_str());
                continue;
            }
            ra_values["body"]["items"][item_count]["name"] = reinforcement->name;
            ra_values["body"]["items"][item_count]["custom_args"] = reinforcement->custom_args;
            ++item_count;
        }
        ra_values["body"]["item_count"] = item_count;
        return this->configuration_->set_custom_ra(StrUtils::json2str(ra_values));
    }
    catch (const std::exception& e)
    {
        LOG_WARNING("%s.", e.what());
        return false;
    }
}

}  // namespace Kiran