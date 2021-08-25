/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-reinforcement.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/daemon/ssr-reinforcement.h"
#include "src/daemon/ssr-plugins.h"
#include "src/daemon/ssr-utils.h"

namespace Kiran
{
SSRReinforcement::SSRReinforcement(const std::string &plugin_id,
                                   const Protocol::Reinforcement &rs) : plugin_id_(plugin_id),
                                                                        config_(rs)
{
    this->reload();
}

std::string SSRReinforcement::get_category_name()
{
    if (this->config_.category().present())
    {
        return this->config_.category().get();
    }
    return std::string();
}

std::string SSRReinforcement::get_label()
{
    return SSRUtils::get_xsd_local_value(this->config_.label());
}

void SSRReinforcement::set_rs(const Protocol::Reinforcement &rs)
{
    this->config_ = rs;
    this->reload();
}

bool SSRReinforcement::match_rules(const Json::Value &values)
{
    try
    {
        for (auto iter : this->rules_)
        {
            RETURN_VAL_IF_TRUE(!values.isMember(iter.first), false);
            RETURN_VAL_IF_TRUE(!iter.second->match(values[iter.first]), false);
        }
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return false;
    }
    return true;
}

void SSRReinforcement::reload()
{
    // 如果加固项未指定分类，则使用插件的分类名
    if (!this->config_.category().present())
    {
        auto plugin = SSRPlugins::get_instance()->get_plugin(this->plugin_id_);
        this->config_.category(plugin->get_category_name());
    }
    this->update_rules();
}

void SSRReinforcement::update_rules()
{
    this->rules_.clear();
    for (const auto &arg : this->config_.arg())
    {
        CONTINUE_IF_TRUE(!arg.rule().present());

        auto rule = SSRRule::create(arg.rule().get());
        if (rule)
        {
            auto iter = this->rules_.emplace(arg.name(), rule);
            if (!iter.second)
            {
                KLOG_WARNING("The rule name %s is repeat.", arg.name().c_str());
            }
        }
        else
        {
            KLOG_WARNING("The rule is created failed. name: %s.", arg.name().c_str());
        }
    }
}
}  // namespace Kiran
