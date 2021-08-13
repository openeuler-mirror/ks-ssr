/**
 * @file          /kiran-sse-manager/src/daemon/sse-reinforcement.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "src/daemon/sse-rule.h"

namespace Kiran
{
struct SSEReinforcementInfo
{
    // 加固项名称
    std::string name;
    // 加固项所属插件名
    std::string plugin_name;
    // 加固项所属分类
    std::string category_name;
    // 加固项标签
    std::string label;
};

class SSEReinforcement
{
public:
    SSEReinforcement() = delete;
    SSEReinforcement(const SSEReinforcementInfo &base_info);
    virtual ~SSEReinforcement(){};

    std::string get_name() { return this->base_info_.name; };
    std::string get_plugin_name() { return this->base_info_.plugin_name; };
    std::string get_category_name() { return this->base_info_.category_name; };
    std::string get_label() { return this->base_info_.label; };

    Json::Value get_custom_args() { return this->custom_args_; };
    Json::Value get_default_args() { return this->default_args_; };
    Json::Value get_layout() { return this->layout_; };

    // 获取加固参数，优先使用自定义参数，如果不存在自定义则使用默认参数
    Json::Value get_args();

    void set_rules(const Json::Value &rules);
    void set_default_args(const Json::Value &default_args) { this->default_args_ = default_args; };
    void set_custom_args(const Json::Value &custom_args) { this->custom_args_ = custom_args; };
    void set_layout(const Json::Value &layout) { this->layout_ = layout; };

    // 判断与规则是否匹配
    bool match_rules(const Json::Value &values);

private:
    // 基本信息
    SSEReinforcementInfo base_info_;
    // 标准的判断规则
    std::map<std::string, std::shared_ptr<SSERule>> rules_;
    // 满足规则的默认参数
    Json::Value default_args_;
    // 自定义加固参数
    Json::Value custom_args_;
    // 前端显示的UI布局
    Json::Value layout_;
};

using SSEReinforcementVec = std::vector<std::shared_ptr<SSEReinforcement>>;
}  // namespace Kiran
