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

std::shared_ptr<SSRReinforcement> SSRPlugins::get_used_reinforcement(const std::string& name)
{
    auto iter = this->used_reinforcements_.find(name);
    RETURN_VAL_IF_TRUE(iter == this->used_reinforcements_.end(), nullptr);
    return iter->second.lock();
}

SSRReinforcementVec SSRPlugins::get_used_reinforcements()
{
    SSRReinforcementVec result;
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

SSRReinforcementVec SSRPlugins::get_used_reinforcements_by_category(const std::string& category_name)
{
    SSRReinforcementVec result;
    for (auto iter : this->used_reinforcements_)
    {
        auto reinforcement = iter.second.lock();
        if (reinforcement && reinforcement->get_category_name() == category_name)
        {
            result.push_back(reinforcement);
        }
    }
    return result;
}

bool SSRPlugins::set_reinforcement_arguments(const std::string& name, const std::string& custom_args)
{
    auto reinforcement = this->get_used_reinforcement(name);
    RETURN_VAL_IF_FALSE(reinforcement, false);
    reinforcement->set_custom_args(custom_args);
    return this->write_ra();
}

std::shared_ptr<SSRReinforcement> SSRPlugins::get_reinforcement(const std::string& name)
{
    auto iter = this->reinforcements_.find(name);
    RETURN_VAL_IF_TRUE(iter == this->reinforcements_.end(), nullptr);
    return iter->second.lock();
}

void SSRPlugins::init()
{
    this->load_plugins();
    this->load_ra();
    this->load_use_reinforcements();
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

    auto reinforcements = plugin->get_reinforcements();
    for (auto reinforcement : reinforcements)
    {
        // KLOG_DEBUG("reinforcement name: %s.", reinforcement->name.c_str());

        auto old_reinforcement = this->get_reinforcement(reinforcement->get_name());
        if (old_reinforcement)
        {
            KLOG_WARNING("The reinforcement %s is conflicted with other plugin. reinforcement name: %s, old plugin: %s, cur plugin: %s.",
                         old_reinforcement->get_name().c_str(),
                         old_reinforcement->get_plugin_name().c_str(),
                         reinforcement->get_plugin_name().c_str());
        }
        else
        {
            this->reinforcements_[reinforcement->get_name()] = reinforcement;
        }
    }
    return true;
}

void SSRPlugins::load_use_reinforcements()
{
    KLOG_PROFILE("");

    auto rs_str = this->configuration_->get_rs();
    auto rs = StrUtils::str2json(rs_str);

    this->used_reinforcements_.clear();

    try
    {
        const auto& rs_items = rs[SSR_JSON_BODY][SSR_JSON_BODY_ITEMS];
        for (uint32_t i = 0; i < rs_items.size(); ++i)
        {
            const auto& rs_item = rs_items[i];
            auto name = rs_item[SSR_JSON_BODY_REINFORCEMENT_NAME].asString();

            // 加固标准中的加固项如果没有插件支持，则加载失败
            auto iter = this->reinforcements_.find(name);
            if (iter == this->reinforcements_.end() || !iter->second.lock())
            {
                KLOG_WARNING("The reinforcement %s is unsupported by any plugin.", name.c_str());
                this->used_reinforcements_.clear();
                return;
            }
            auto reinforcement = iter->second.lock();
            reinforcement->set_rules(rs_item[SSR_JSON_BODY_RULES]);
            reinforcement->set_default_args(rs_item[SSR_JSON_BODY_REINFORCEMENT_DEFAULT_ARGS]);
            reinforcement->set_layout(rs_item[SSR_JSON_BODY_REINFORCEMENT_LAYOUT]);
            this->used_reinforcements_.emplace(name, reinforcement);
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        this->used_reinforcements_.clear();
    }
}

void SSRPlugins::load_ra()
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
                    KLOG_WARNING("The reinforcement %s isn't found.", name.c_str());
                    continue;
                }
                auto reinforcement = iter->second.lock();
                reinforcement->set_custom_args(ra_item[SSR_JSON_BODY_REINFORCEMENT_CUSTOM_ARGS]);
            }
        }
        catch (const std::exception& e)
        {
            KLOG_WARNING("%s.", e.what());
        }
    } while (0);
}

bool SSRPlugins::write_ra()
{
    try
    {
        Json::Value ra_values;
        ra_values[SSR_JSON_HEAD][SSR_JSON_HEAD_VERSION] = PROJECT_VERSION;
        int32_t item_count = 0;

        for (auto iter : this->reinforcements_)
        {
            auto reinforcement = iter.second.lock();
            if (!reinforcement)
            {
                KLOG_WARNING("The reinforcement %s isn't found.", iter.first.c_str());
                continue;
            }
            ra_values[SSR_JSON_BODY][SSR_JSON_BODY_ITEMS][item_count][SSR_JSON_BODY_REINFORCEMENT_NAME] = reinforcement->get_name();
            ra_values[SSR_JSON_BODY][SSR_JSON_BODY_ITEMS][item_count][SSR_JSON_BODY_REINFORCEMENT_CUSTOM_ARGS] = reinforcement->get_custom_args();
            ++item_count;
        }
        ra_values[SSR_JSON_BODY][SSR_JSON_BODY_REINFORCEMENT_COUNT] = item_count;
        return this->configuration_->set_custom_ra(StrUtils::json2str(ra_values));
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return false;
    }
}

}  // namespace Kiran