/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#include "plugin.h"

namespace KS
{
namespace BRDaemon
{
Plugin::Plugin(const QString& conf_path) : conf_path_(conf_path)
{
}

Plugin::~Plugin()
{
}

bool Plugin::init()
{
    KLOG_DEBUG() << "plugin config path: " << this->conf_path_.toLocal8Bit();

    try
    {
        this->plugin_config_ = Protocol::br_plugin(this->conf_path_.toStdString(), xml_schema::Flags::dont_validate);

        // 判断插件是否启用
        if (!this->plugin_config_->available())
        {
            KLOG_DEBUG("Plugin %s is unavailable.", this->plugin_config_->name().c_str());
            return false;
        }
    }
    catch (const xml_schema::Exception& e)
    {
        KLOG_WARNING("Failed to load file: %s: %s", this->conf_path_.toLatin1(), e.what());
        return false;
    }

    // 加载插件
    RETURN_VAL_IF_FALSE(this->loadPluginModule(), false);

    return true;
}

std::vector<std::string> Plugin::getReinforcementNames()
{
    std::vector<std::string> names;
    const auto& reinforcements = this->plugin_config_->reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        names.push_back((*iter).name());
    }
    return names;
}

const Protocol::Reinforcement* Plugin::getReinforcementConfig(const std::string& name)
{
    const auto& reinforcements = this->plugin_config_->reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        RETURN_VAL_IF_TRUE((*iter).name() == name, &(*iter));
    }
    return NULL;
}

bool Plugin::loadPluginModule()
{
    KLOG_DEBUG("");

    auto dirname = QFileInfo(this->conf_path_).fileName();
    switch (this->plugin_config_->language_type())
    {
    case Protocol::LanguageType::Value::cpp:
    {
        auto so_path = QDir::cleanPath(dirname + '/' + "lib" + QString::fromStdString(this->plugin_config_->name()) + ".so");
        this->loader_ = std::make_shared<PluginCPPLoader>(so_path);
        return this->loader_->load();
    }
    case Protocol::LanguageType::Value::python:
    {
        this->loader_ = std::make_shared<PluginPythonLoader>(QString::fromStdString(fmt::format("br.{0}", this->plugin_config_->name())));
        return this->loader_->load();
    }
    default:
        KLOG_WARNING("Unsupported language type: %d.", this->plugin_config_->language_type());
        return false;
    }
}
}  // namespace BRDaemon
}  // namespace KS
