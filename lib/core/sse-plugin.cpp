/**
 * @file          /kiran-sse-manager/lib/core/sse-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/core/sse-plugin.h"

namespace Kiran
{
#define SSE_PLUGIN_GROUP_NAME "plugin entry"
#define SSE_PLUGIN_KEY_NAME "name"
#define SSE_PLUGIN_KEY_LABEL "label"
#define SSE_PLUGIN_KEY_LANGUAGE_TYPE "language_type"
#define SSE_PLUGIN_KEY_AVAILABLE "available"

#define SSE_PLUGIN_KEY_REINFORCEMENT_ITEMS "items"

#define SSE_REINFORCEMENT_KEY_CATEGORY_NAME "category_name"
#define SSE_REINFORCEMENT_KEY_LABEL "label"

SSEPlugin::SSEPlugin(const std::string& conf_path) : conf_path_(conf_path)
{
}

SSEPlugin::~SSEPlugin()
{
}

bool SSEPlugin::init()
{
    KLOG_DEBUG("plugin config path: %s.", this->conf_path_.c_str());

    Glib::KeyFile keyfile;

    // 加载插件配置
    try
    {
        keyfile.load_from_file(this->conf_path_);

        this->plugin_info_.name = keyfile.get_string(SSE_PLUGIN_GROUP_NAME, SSE_PLUGIN_KEY_NAME);

        auto available = keyfile.get_boolean(SSE_PLUGIN_GROUP_NAME, SSE_PLUGIN_KEY_AVAILABLE);
        if (!available)
        {
            KLOG_DEBUG("Plugin %s is unavailable.", this->plugin_info_.name.c_str());
            return false;
        }

        auto language_type_str = keyfile.get_string(SSE_PLUGIN_GROUP_NAME, SSE_PLUGIN_KEY_LANGUAGE_TYPE);
        switch (shash(language_type_str.c_str()))
        {
        case "cpp"_hash:
            this->plugin_info_.language_type = SSEPluginLanguageType::SSE_PLUGIN_LANGUAGE_TYPE_CPP;
            break;
        case "python"_hash:
            this->plugin_info_.language_type = SSEPluginLanguageType::SSE_PLUGIN_LANGUAGE_TYPE_PYTHON;
            break;
        default:
            KLOG_WARNING("Unsupported language type: %s.", language_type_str.c_str());
            return false;
        }

        this->plugin_info_.reinforcements_name = keyfile.get_string_list(SSE_PLUGIN_GROUP_NAME, SSE_PLUGIN_KEY_REINFORCEMENT_ITEMS);
        KLOG_DEBUG("the reinforcements of plugin: %s.", StrUtils::join(this->plugin_info_.reinforcements_name, ",").c_str());
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return false;
    }

    // 加载插件
    RETURN_VAL_IF_FALSE(this->load_plugin_module(), false);

    // 加载插件的加固项
    for (const auto& reinforcement_name : this->plugin_info_.reinforcements_name)
    {
        RETURN_VAL_IF_FALSE(this->load_reinforcement(keyfile, reinforcement_name), false);
    }

    return true;
}

bool SSEPlugin::load_plugin_module()
{
    KLOG_PROFILE("");

    auto dirname = Glib::path_get_dirname(this->conf_path_);
    switch (this->plugin_info_.language_type)
    {
    case SSEPluginLanguageType::SSE_PLUGIN_LANGUAGE_TYPE_CPP:
    {
        auto so_filename = Glib::build_filename(dirname, "cpp", "lib" + this->plugin_info_.name + ".so");
        this->loader_ = std::make_shared<SSEPluginCPPLoader>(so_filename);
        return this->loader_->load();
    }
    // case SSEPluginLanguageType::SSE_PLUGIN_LANGUAGE_TYPE_PYTHON:
    default:
        KLOG_WARNING("Unsupported language type: %d.", this->plugin_info_.language_type);
        return false;
    }
}

bool SSEPlugin::load_reinforcement(const Glib::KeyFile& keyfile, const std::string& reinforcement_name)
{
    KLOG_PROFILE("reinforcement name: %s.", reinforcement_name.c_str());

    auto reinforcement = std::make_shared<SSEReinforcement>();

    try
    {
        reinforcement->name = reinforcement_name;
        reinforcement->plugin_name = this->plugin_info_.name;
        reinforcement->category_name = keyfile.get_string(reinforcement_name, SSE_REINFORCEMENT_KEY_CATEGORY_NAME);
        reinforcement->label = keyfile.get_locale_string(reinforcement_name, SSE_REINFORCEMENT_KEY_LABEL);

        return this->add_reinforcement(reinforcement);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
    }

    return false;
}

bool SSEPlugin::add_reinforcement(std::shared_ptr<SSEReinforcement> reinforcement)
{
    RETURN_VAL_IF_FALSE(reinforcement, false);

    auto iter = this->reinforcements_.emplace(reinforcement->name, reinforcement);
    if (!iter.second)
    {
        KLOG_WARNING("The reinforcement is already exist. name: %s.", reinforcement->name.c_str());
        return false;
    }
    return true;
}

}  // namespace Kiran
