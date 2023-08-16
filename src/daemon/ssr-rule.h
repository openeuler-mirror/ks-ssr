/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-rule.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include <json/json.h>
#include "lib/base/base.h"
#include "src/daemon/ssr-rs-config.hxx"

namespace Kiran
{
// enum SSRRuleType
// {
//     SSR_RULE_TYPE_NONE = 0,
//     // 值是否在一个连续范围内
//     SSR_RULE_TYPE_RANGE,
//     // 值是否在枚举集合中
//     SSR_RULE_TYPE_ENUM,
// };

class SSRRule
{
public:
    // 规则类型
    virtual RS::SSRRuleType get_type() { return RS::SSRRuleType::Value::NONE; };
    // 判断该值是否符合规则
    virtual bool match(const Json::Value &value) { return false; };

    static std::shared_ptr<SSRRule> create(const Json::Value &rule);
    static std::shared_ptr<SSRRule> create(const RS::SSRRSRule &rule);

protected:
    enum JsonCmpResult
    {
        // 未知
        JSON_CMP_RESULT_UNKNOWN,
        // 小于
        JSON_CMP_RESULT_LESS,
        // 等于
        JSON_CMP_RESULT_EQUAL,
        // 大于
        JSON_CMP_RESULT_GREATER,
    };
    // 比较两个值
    JsonCmpResult json_value_cmp(const Json::Value &v1, const Json::Value &v2);
};

using SSRRuleVec = std::vector<std::shared_ptr<SSRRule>>;

class SSRRuleRange : public SSRRule
{
public:
    // 如果min_value为空，则表示无限小，如果max_value为空，则表示无限大
    SSRRuleRange(const Json::Value &min_value,
                 const Json::Value &max_value);
    virtual ~SSRRuleRange(){};

    // 规则类型
    virtual RS::SSRRuleType get_type() override { return RS::SSRRuleType::Value::RANGE; };
    // 判断该值是否符合规则
    virtual bool match(const Json::Value &value) override;

private:
    Json::Value min_value_;
    Json::Value max_value_;
    Json::ValueType value_type_;
};

class SSRRuleFixed : public SSRRuleRange
{
public:
    // 如果min_value为空，则表示无限小，如果max_value为空，则表示无限大
    SSRRuleFixed(const Json::Value &value);
    virtual ~SSRRuleFixed(){};

    // 规则类型
    virtual RS::SSRRuleType get_type() override { return RS::SSRRuleType::Value::RANGE; };
};

class SSRRuleEnum : public SSRRule
{
public:
    SSRRuleEnum(const std::vector<Json::Value> &values);
    virtual ~SSRRuleEnum(){};

    // 规则类型
    virtual RS::SSRRuleType get_type() override { return RS::SSRRuleType::Value::ENUM; };
    // 判断该值是否符合规则
    virtual bool match(const Json::Value &value) override;

private:
    std::vector<Json::Value> enum_values_;
};

}  // namespace Kiran
