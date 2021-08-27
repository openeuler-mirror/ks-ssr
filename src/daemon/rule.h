/**
 * @file          /kiran-ssr-manager/src/daemon/rule.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include <json/json.h>
#include "lib/base/base.h"
#include "src/daemon/ssr-protocol.hxx"

namespace Kiran
{
namespace Daemon
{
class Rule
{
public:
    // 规则类型
    virtual Protocol::RuleType get_type() { return Protocol::RuleType::Value::NONE; };
    // 判断该值是否符合规则
    virtual bool match(const Json::Value &value) { return false; };

    static std::shared_ptr<Rule> create(const Protocol::Rule &rule);

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

using RuleVec = std::vector<std::shared_ptr<Rule>>;

class RuleRange : public Rule
{
public:
    // 如果min_value为空，则表示无限小，如果max_value为空，则表示无限大
    RuleRange(const Json::Value &min_value,
              const Json::Value &max_value);
    virtual ~RuleRange(){};

    // 规则类型
    virtual Protocol::RuleType get_type() override { return Protocol::RuleType::Value::RANGE; };
    // 判断该值是否符合规则
    virtual bool match(const Json::Value &value) override;

private:
    Json::Value min_value_;
    Json::Value max_value_;
    Json::ValueType value_type_;
};

class RuleFixed : public RuleRange
{
public:
    // 如果min_value为空，则表示无限小，如果max_value为空，则表示无限大
    RuleFixed(const Json::Value &value);
    virtual ~RuleFixed(){};

    // 规则类型
    virtual Protocol::RuleType get_type() override { return Protocol::RuleType::Value::RANGE; };
};

class RuleEnum : public Rule
{
public:
    RuleEnum(const std::vector<Json::Value> &values);
    virtual ~RuleEnum(){};

    // 规则类型
    virtual Protocol::RuleType get_type() override { return Protocol::RuleType::Value::ENUM; };
    // 判断该值是否符合规则
    virtual bool match(const Json::Value &value) override;

private:
    std::vector<Json::Value> enum_values_;
};

}  // namespace Daemon
}  // namespace Kiran
