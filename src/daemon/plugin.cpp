/**
 * @file          /ks-ssr-manager/src/daemon/plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/plugin.h"

namespace KS
{
namespace Daemon
{
Plugin::Plugin(const std::string& conf_path) : conf_path_(conf_path)
{
}

Plugin::~Plugin()
{
}

bool Plugin::init()
{
    KLOG_DEBUG("plugin config path: %s.", this->conf_path_.c_str());

    try
    {
        this->plugin_config_ = Protocol::ssr_plugin(this->conf_path_, xml_schema::Flags::dont_validate);

        // 判断插件是否启用
        if (!this->plugin_config_->available())
        {
            KLOG_DEBUG("Plugin %s is unavailable.", this->plugin_config_->name().c_str());
            return false;
        }
    }
    catch (const xml_schema::Exception& e)
    {
        KLOG_WARNING("Failed to load file: %s: %s", this->conf_path_.c_str(), e.what());
        return false;
    }

    // 加载插件
    RETURN_VAL_IF_FALSE(this->load_plugin_module(), false);

    return true;
}

std::vector<std::string> Plugin::get_reinforcement_names()
{
    std::vector<std::string> names;
    const auto& reinforcements = this->plugin_config_->reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        names.push_back((*iter).name());
    }
    return names;
}

const Protocol::Reinforcement* Plugin::get_reinforcement_config(const std::string& name)
{
    const auto& reinforcements = this->plugin_config_->reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        RETURN_VAL_IF_TRUE((*iter).name() == name, &(*iter));
    }
    return NULL;
}

bool Plugin::load_plugin_module()
{
    KLOG_PROFILE("");

    auto dirname = Glib::path_get_dirname(this->conf_path_);
    switch (this->plugin_config_->language_type())
    {
    case Protocol::LanguageType::Value::cpp:
    {
        auto so_path = Glib::build_filename(dirname, "lib" + this->plugin_config_->name() + ".so");
        this->loader_ = std::make_shared<PluginCPPLoader>(so_path);
        return this->loader_->load();
    }
    case Protocol::LanguageType::Value::python:
    {
        this->loader_ = std::make_shared<PluginPythonLoader>(fmt::format("ssr.{0}", this->plugin_config_->name()));
        return this->loader_->load();
    }
    default:
        KLOG_WARNING("Unsupported language type: %d.", (int32_t)this->plugin_config_->language_type());
        return false;
    }
}
}  // namespace Daemon
}  // namespace KS
