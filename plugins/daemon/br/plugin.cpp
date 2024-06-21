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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#include "plugin.h"

namespace KS
{
namespace BR
{
Plugin::Plugin(const QString& confPath)
    : m_confPath(confPath)
{
}

Plugin::~Plugin()
{
}

bool Plugin::init()
{
    KLOG_DEBUG() << "plugin config path: " << this->m_confPath.toLocal8Bit();

    try
    {
        this->m_pluginConfig = Protocol::br_plugin(this->m_confPath.toStdString(), xml_schema::Flags::dont_validate);

        // 判断插件是否启用
        if (!this->m_pluginConfig->available())
        {
            KLOG_DEBUG("Plugin %s is unavailable.", this->m_pluginConfig->name().c_str());
            return false;
        }
    }
    catch (const xml_schema::Exception& e)
    {
        KLOG_WARNING() << "Failed to load file: %s" << this->m_confPath.toLatin1() << ": " << e.what();
        return false;
    }

    // 加载插件
    RETURN_VAL_IF_FALSE(this->loadPluginModule(), false);

    return true;
}

std::vector<std::string> Plugin::getReinforcementNames()
{
    std::vector<std::string> names;
    const auto& reinforcements = this->m_pluginConfig->reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        names.push_back((*iter).name());
    }
    return names;
}

const Protocol::Reinforcement* Plugin::getReinforcementConfig(const std::string& name)
{
    const auto& reinforcements = this->m_pluginConfig->reinforcement();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        RETURN_VAL_IF_TRUE((*iter).name() == name, &(*iter));
    }
    return NULL;
}

bool Plugin::loadPluginModule()
{
    KLOG_DEBUG("Plugin::loadPluginModule");

    auto dirname = QFileInfo(this->m_confPath).absolutePath();
    switch (this->m_pluginConfig->language_type())
    {
    case Protocol::LanguageType::Value::cpp:
    {
        auto soPath = QDir::cleanPath(dirname + '/' + "lib" + QString::fromStdString(this->m_pluginConfig->name()) + ".so");
        this->m_loader = std::make_shared<PluginCPPLoader>(soPath);
        return this->m_loader->load();
    }
    case Protocol::LanguageType::Value::python:
    {
        this->m_loader = std::make_shared<PluginPythonLoader>(QString::fromStdString("br." + this->m_pluginConfig->name()));
        return this->m_loader->load();
    }
    default:
        KLOG_WARNING() << "Unsupported language type: " << this->m_pluginConfig->language_type();
        return false;
    }
}
}  // namespace BR
}  // namespace KS
