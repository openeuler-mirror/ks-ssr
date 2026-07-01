/**
 * @file          /kiran-sse-manager/src/daemon/sse-reinforcement.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/daemon/sse-reinforcement.h"

namespace Kiran
{
SSEReinforcement::SSEReinforcement(const SSEReinforcementInfo &base_info) : base_info_(base_info)
{
}

Json::Value SSEReinforcement::get_args()
{
    if (!this->custom_args_.isNull())
    {
        return this->custom_args_;
    }
    return this->default_args_;
}

void SSEReinforcement::set_rules(const Json::Value &rules)
{
    try
    {
        this->rules_.clear();
        for (auto member : rules.getMemberNames())
        {
            auto rule = SSERule::create(rules[member]);
            if (rule)
            {
                auto iter = this->rules_.emplace(member, rule);
                if (!iter.second)
                {
                    KLOG_WARNING("The rule name %s is repeat.", member.c_str());
                }
            }
            else
            {
                KLOG_WARNING("The rule is created failed. name: %s.", member.c_str());
            }
        }
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
    }
}

bool SSEReinforcement::match_rules(const Json::Value &values)
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
}  // namespace Kiran
