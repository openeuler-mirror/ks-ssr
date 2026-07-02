/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-reinforcement.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "src/daemon/ssr-plugin-config.hxx"
#include "src/daemon/ssr-rs-config.hxx"
#include "src/daemon/ssr-rule.h"

namespace Kiran
{
class SSRReinforcement
{
public:
    SSRReinforcement() = delete;
    SSRReinforcement(const std::string &plugin_name,
                     const Plugin::ReinforcementConfig &base_info,
                     const RS::SSRRSReinforcement &rs);
    virtual ~SSRReinforcement(){};

    std::string get_name() { return this->base_info_.name(); };
    std::string get_plugin_name() { return this->plugin_name_; };
    std::string get_category_name();
    std::string get_label();

    void set_rs(const RS::SSRRSReinforcement &rs);
    const RS::SSRRSReinforcement &get_rs() { return this->rs_; };

    // 判断与规则是否匹配
    bool match_rules(const Json::Value &values);

private:
    void update_rules();

private:
    // 插件所属插件
    std::string plugin_name_;
    // 加固项的基本信息
    Plugin::ReinforcementConfig base_info_;
    // 加固项的加固标准
    RS::SSRRSReinforcement rs_;
    // 标准的判断规则
    std::map<std::string, std::shared_ptr<SSRRule>> rules_;
};

using SSRReinforcementVec = std::vector<std::shared_ptr<SSRReinforcement>>;
}  // namespace Kiran
