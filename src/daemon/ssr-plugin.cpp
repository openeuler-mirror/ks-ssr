/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/ssr-plugin.h"

namespace Kiran
{
#define SSR_PLUGIN_GROUP_NAME "plugin entry"
#define SSR_PLUGIN_KEY_NAME "name"
#define SSR_PLUGIN_KEY_LABEL "label"
#define SSR_PLUGIN_KEY_LANGUAGE_TYPE "language_type"
#define SSR_PLUGIN_KEY_AVAILABLE "available"

#define SSR_PLUGIN_KEY_REINFORCEMENT_ITEMS "items"

#define SSR_REINFORCEMENT_KEY_CATEGORY_NAME "category_name"
#define SSR_REINFORCEMENT_KEY_LABEL "label"

SSRPlugin::SSRPlugin(const std::string& conf_path) : conf_path_(conf_path)
{
}

SSRPlugin::~SSRPlugin()
{
}

bool SSRPlugin::init()
{
    KLOG_DEBUG("plugin config path: %s.", this->conf_path_.c_str());

    Glib::KeyFile keyfile;

    // 加载插件配置
    try
    {
        keyfile.load_from_file(this->conf_path_);

        this->plugin_info_.name = keyfile.get_string(SSR_PLUGIN_GROUP_NAME, SSR_PLUGIN_KEY_NAME);

        auto available = keyfile.get_boolean(SSR_PLUGIN_GROUP_NAME, SSR_PLUGIN_KEY_AVAILABLE);
        if (!available)
        {
            KLOG_DEBUG("Plugin %s is unavailable.", this->plugin_info_.name.c_str());
            return false;
        }

        auto language_type_str = keyfile.get_string(SSR_PLUGIN_GROUP_NAME, SSR_PLUGIN_KEY_LANGUAGE_TYPE);
        switch (shash(language_type_str.c_str()))
        {
        case "cpp"_hash:
            this->plugin_info_.language_type = SSRPluginLanguageType::SSR_PLUGIN_LANGUAGE_TYPE_CPP;
            break;
        case "python"_hash:
            this->plugin_info_.language_type = SSRPluginLanguageType::SSR_PLUGIN_LANGUAGE_TYPE_PYTHON;
            break;
        default:
            KLOG_WARNING("Unsupported language type: %s.", language_type_str.c_str());
            return false;
        }

        this->plugin_info_.reinforcements_name = keyfile.get_string_list(SSR_PLUGIN_GROUP_NAME, SSR_PLUGIN_KEY_REINFORCEMENT_ITEMS);
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

bool SSRPlugin::load_plugin_module()
{
    KLOG_PROFILE("");

    auto dirname = Glib::path_get_dirname(this->conf_path_);
    switch (this->plugin_info_.language_type)
    {
    case SSRPluginLanguageType::SSR_PLUGIN_LANGUAGE_TYPE_CPP:
    {
        auto so_filename = Glib::build_filename(dirname, "cpp", "lib" + this->plugin_info_.name + ".so");
        this->loader_ = std::make_shared<SSRPluginCPPLoader>(so_filename);
        return this->loader_->load();
    }
    // case SSRPluginLanguageType::SSR_PLUGIN_LANGUAGE_TYPE_PYTHON:
    default:
        KLOG_WARNING("Unsupported language type: %d.", this->plugin_info_.language_type);
        return false;
    }
}

bool SSRPlugin::load_reinforcement(const Glib::KeyFile& keyfile, const std::string& reinforcement_name)
{
    KLOG_PROFILE("reinforcement name: %s.", reinforcement_name.c_str());

    try
    {
        auto reinforcement = std::make_shared<SSRReinforcement>(
            SSRReinforcementInfo{.name = reinforcement_name,
                                 .plugin_name = this->plugin_info_.name,
                                 .category_name = keyfile.get_string(reinforcement_name, SSR_REINFORCEMENT_KEY_CATEGORY_NAME),
                                 .label = keyfile.get_locale_string(reinforcement_name, SSR_REINFORCEMENT_KEY_LABEL)});

        return this->add_reinforcement(reinforcement);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
    }

    return false;
}

bool SSRPlugin::add_reinforcement(std::shared_ptr<SSRReinforcement> reinforcement)
{
    RETURN_VAL_IF_FALSE(reinforcement, false);

    auto iter = this->reinforcements_.emplace(reinforcement->get_name(), reinforcement);
    if (!iter.second)
    {
        KLOG_WARNING("The reinforcement is already exist. name: %s.", reinforcement->get_name().c_str());
        return false;
    }
    return true;
}

}  // namespace Kiran
