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
SSRReinforcement::SSRReinforcement(const std::string &plugin_name,
                                   const Plugin::ReinforcementConfig &base_info,
                                   const RS::SSRRSReinforcement &rs) : plugin_name_(plugin_name),
                                                                       base_info_(base_info),
                                                                       rs_(rs)
{
    this->update_rules();
}

std::string SSRReinforcement::get_category_name()
{
    // 如果加固项指定了分类则使用加固项的分类名，否则使用插件的分类名
    if (this->base_info_.category().present())
    {
        return this->base_info_.category().get();
    }

    auto plugin = SSRPlugins::get_instance()->get_plugin(this->plugin_name_);
    RETURN_VAL_IF_FALSE(plugin, std::string());
    return plugin->get_category_name();
}

std::string SSRReinforcement::get_label()
{
    return SSRUtils::get_xsd_local_value(this->base_info_.label());
}

void SSRReinforcement::set_rs(const RS::SSRRSReinforcement &rs)
{
    this->rs_ = rs;
    this->update_rules();
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

void SSRReinforcement::update_rules()
{
    this->rules_.clear();
    for (const auto &arg : this->rs_.arg())
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
