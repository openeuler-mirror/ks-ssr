/**
 * @file          /kiran-sse-manager/src/daemon/sse-rule.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include <json/json.h>
#include "lib/base/base.h"

namespace Kiran
{
enum SSERuleType
{
    SSE_RULE_TYPE_NONE = 0,
    // 值是否在一个连续范围内
    SSE_RULE_TYPE_RANGE,
    // 值是否在枚举集合中
    SSE_RULE_TYPE_ENUM,
};

class SSERule
{
public:
    // 规则类型
    virtual SSERuleType get_type() { return SSERuleType::SSE_RULE_TYPE_NONE; };
    // 判断该值是否符合规则
    virtual bool match(const Json::Value &value) { return false; };

    static std::shared_ptr<SSERule> create(const Json::Value &rule);

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

using SSERuleVec = std::vector<std::shared_ptr<SSERule>>;

class SSERuleRange : public SSERule
{
public:
    // 如果min_value为空，则表示无限小，如果max_value为空，则表示无限大
    SSERuleRange(const Json::Value &min_value,
                 const Json::Value &max_value);
    virtual ~SSERuleRange(){};

    // 规则类型
    virtual SSERuleType get_type() override { return SSERuleType::SSE_RULE_TYPE_RANGE; };
    // 判断该值是否符合规则
    virtual bool match(const Json::Value &value) override;

private:
    Json::Value min_value_;
    Json::Value max_value_;
    Json::ValueType value_type_;
};

class SSERuleEnum : public SSERule
{
public:
    SSERuleEnum(const std::vector<Json::Value> &values);
    virtual ~SSERuleEnum(){};

    // 规则类型
    virtual SSERuleType get_type() override { return SSERuleType::SSE_RULE_TYPE_ENUM; };
    // 判断该值是否符合规则
    virtual bool match(const Json::Value &value) override;

private:
    std::vector<Json::Value> enum_values_;
};

}  // namespace Kiran
