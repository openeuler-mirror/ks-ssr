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

#include "reinforcement.h"
#include "context.h"
#include "plugins.h"
#include "utils.h"

namespace KS
{
namespace BR
{
Reinforcement::Reinforcement(const QString &pluginID,
                             const Protocol::Reinforcement &rs)
    : m_pluginID(pluginID),
      m_config(rs)
{
    this->reload();
}

QString Reinforcement::getCategoryName()
{
    if (this->m_config.category().present())
    {
        return QString::fromStdString(this->m_config.category().get());
    }
    return QString();
}

QString Reinforcement::getLabel()
{
    return Utils::getXsdLocalValue(this->m_config.label());
}

void Reinforcement::setRs(const Protocol::Reinforcement &rs)
{
    this->m_config = rs;
    this->reload();
}

bool Reinforcement::matchRules(const QJsonObject &values)
{
    // std::map<std::string, std::shared_ptr<Rule>>::iterator iter;
    for (auto iter = this->m_rules.begin(); iter != this->m_rules.end(); iter++)
    {
        RETURN_VAL_IF_TRUE(!values.contains(iter.key()), false);
        RETURN_VAL_IF_TRUE(!iter.value()->match(values[iter.key()]), false);
    }
    return true;
}

void Reinforcement::reload()
{
    // 如果加固项未指定分类，则使用插件的分类名
    if (!this->m_config.category().present())
    {
        auto plugin = Plugins::getInstance()->getPlugin(this->m_pluginID);
        this->m_config.category(plugin->getCategoryName());
    }
    this->updateRules();
}

void Reinforcement::updateRules()
{
    this->m_rules.clear();

    //    this->config_.arg().
    //    Protocol::Reinforcement::
    for (auto arg = this->m_config.arg().begin(); arg != this->m_config.arg().end(); ++arg)
    {
        CONTINUE_IF_TRUE(!arg->rule().present());

        auto rule = Rule::create(arg->rule().get());
        if (rule)
        {
            if (this->m_rules.find(QString::fromStdString(arg->name())) != this->m_rules.end())
            {
                KLOG_WARNING("The rule name %s is repeat.", arg->name().c_str());
            }
            else
            {
                this->m_rules[QString::fromStdString(arg->name())] = rule;
            }
        }
        else
        {
            KLOG_WARNING("The rule is created failed. name: %s.", arg->name().c_str());
        }
    }
}
}  // namespace BR
}  // namespace KS