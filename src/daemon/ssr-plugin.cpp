/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/ssr-plugin.h"

namespace Kiran
{
using namespace Plugin;

SSRPlugin::SSRPlugin(const std::string& conf_path) : conf_path_(conf_path)
{
}

SSRPlugin::~SSRPlugin()
{
}

bool SSRPlugin::init()
{
    KLOG_DEBUG("plugin config path: %s.", this->conf_path_.c_str());

    try
    {
        this->plugin_config_ = plugin_config(this->conf_path_, xml_schema::Flags::dont_validate);

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

std::vector<std::string> SSRPlugin::get_reinforcement_names()
{
    std::vector<std::string> names;
    for (const auto& reinforcement_config : this->plugin_config_->reinforcement())
    {
        names.push_back(reinforcement_config.name());
    }
    return names;
}

const ReinforcementConfig* SSRPlugin::get_reinforcement_config(const std::string& name)
{
    for (const auto& reinforcement_config : this->plugin_config_->reinforcement())
    {
        RETURN_VAL_IF_TRUE(reinforcement_config.name() == name, &reinforcement_config);
    }
    return nullptr;
}

bool SSRPlugin::load_plugin_module()
{
    KLOG_PROFILE("");

    auto dirname = Glib::path_get_dirname(this->conf_path_);
    switch (this->plugin_config_->language_type())
    {
    case PluginConfig::Language_typeType::Value::cpp:
    {
        auto so_filename = Glib::build_filename(dirname, "cpp", "lib" + this->plugin_config_->name() + ".so");
        this->loader_ = std::make_shared<SSRPluginCPPLoader>(so_filename);
        return this->loader_->load();
    }
    // case PluginConfig::Language_typeType::Value::python:
    default:
        KLOG_WARNING("Unsupported language type: %d.", (int32_t)this->plugin_config_->language_type());
        return false;
    }
}

}  // namespace Kiran
