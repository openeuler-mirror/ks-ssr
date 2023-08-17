/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-reinforcement.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "src/daemon/ssr-protocol.hxx"
#include "src/daemon/ssr-rule.h"

namespace Kiran
{
class SSRReinforcement
{
public:
    SSRReinforcement() = delete;
    SSRReinforcement(const std::string &plugin_name,
                     const Protocol::Reinforcement &rs);
    virtual ~SSRReinforcement(){};

    std::string get_name() { return this->config_.name(); };
    std::string get_plugin_name() { return this->plugin_name_; };
    std::string get_category_name();
    std::string get_label();

    void set_rs(const Protocol::Reinforcement &rs);
    const Protocol::Reinforcement &get_rs() { return this->config_; };

    // 判断与规则是否匹配
    bool match_rules(const Json::Value &values);

private:
    void reload();
    void update_rules();

private:
    // 插件所属插件
    std::string plugin_name_;
    // 加固项的加固标准
    Protocol::Reinforcement config_;
    // 标准的判断规则
    std::map<std::string, std::shared_ptr<SSRRule>> rules_;
};

using SSRReinforcementVec = std::vector<std::shared_ptr<SSRReinforcement>>;
}  // namespace Kiran
