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

#include "reinforcement.h"
#include "plugins.h"
#include "utils.h"

namespace KS
{
namespace BRDaemon
{
Reinforcement::Reinforcement(const QString &plugin_id,
                             const Protocol::Reinforcement &rs) : plugin_id_(plugin_id),
                                                                  config_(rs)
{
    this->reload();
}

QString Reinforcement::getCategoryName()
{
    if (this->config_.category().present())
    {
        return QString::fromStdString(this->config_.category().get());
    }
    return QString();
}

QString Reinforcement::getLabel()
{
    return Utils::getXsdLocalValue(this->config_.label());
}

void Reinforcement::setRs(const Protocol::Reinforcement &rs)
{
    this->config_ = rs;
    this->reload();
}

bool Reinforcement::matchRules(const QJsonObject &values)
{
    // std::map<std::string, std::shared_ptr<Rule>>::iterator iter;
    for (auto iter = this->rules_.begin(); iter != this->rules_.end(); iter++)
    {
        RETURN_VAL_IF_TRUE(!values.contains(iter.key()), false);
        RETURN_VAL_IF_TRUE(!iter.value()->match(values[iter.key()]), false);
    }
    return true;
}

void Reinforcement::reload()
{
    // 如果加固项未指定分类，则使用插件的分类名
    if (!this->config_.category().present())
    {
        auto plugin = Plugins::getInstance()->getPlugin(this->plugin_id_);
        this->config_.category(plugin->getCategoryName());
    }
    this->updateRules();
}

void Reinforcement::updateRules()
{
    this->rules_.clear();

    //    this->config_.arg().
    //    Protocol::Reinforcement::
    for (auto arg = this->config_.arg().begin(); arg != this->config_.arg().end(); ++arg)
    {
        CONTINUE_IF_TRUE(!arg->rule().present());

        auto rule = Rule::create(arg->rule().get());
        if (rule)
        {
            if (this->rules_.find(QString::fromStdString(arg->name())) != this->rules_.end())
            {
                KLOG_WARNING("The rule name %s is repeat.", arg->name().c_str());
            }
            else
            {
                this->rules_[QString::fromStdString(arg->name())] = rule;
            }
        }
        else
        {
            KLOG_WARNING("The rule is created failed. name: %s.", arg->name().c_str());
        }
    }
}
}  // namespace BRDaemon
}  // namespace KS