/**
 * @file          /kiran-sse-manager/src/daemon/sse-rule.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/daemon/sse-rule.h"

namespace Kiran
{
std::shared_ptr<SSERule> SSERule::create(const Json::Value &rule)
{
    RETURN_VAL_IF_TRUE(!rule[SSE_JSON_BODY_RULES_TYPE].isInt(), nullptr);
    auto type = rule[SSE_JSON_BODY_RULES_TYPE].asInt();

    switch (type)
    {
    case SSERuleType::SSE_RULE_TYPE_RANGE:
    {
        RETURN_VAL_IF_TRUE(!rule.isMember(SSE_JSON_BODY_RULES_MIN_VALUE) && !rule.isMember(SSE_JSON_BODY_RULES_MAX_VALUE), nullptr);
        if (rule.isMember(SSE_JSON_BODY_RULES_MIN_VALUE) &&
            rule.isMember(SSE_JSON_BODY_RULES_MAX_VALUE) &&
            rule[SSE_JSON_BODY_RULES_MIN_VALUE].type() != rule[SSE_JSON_BODY_RULES_MAX_VALUE].type())
        {
            KLOG_WARNING("The type of min_value and max_value is not equal.");
            return nullptr;
        }
        return std::make_shared<SSERuleRange>(rule[SSE_JSON_BODY_RULES_MIN_VALUE], rule[SSE_JSON_BODY_RULES_MAX_VALUE]);
    }
    default:
        break;
    }
    return nullptr;
}

SSERule::JsonCmpResult SSERule::json_value_cmp(const Json::Value &v1, const Json::Value &v2)
{
    RETURN_VAL_IF_TRUE(v1.isNull() && v2.isNull(), JsonCmpResult::JSON_CMP_RESULT_EQUAL);
    RETURN_VAL_IF_TRUE(v1.isNull(), JsonCmpResult::JSON_CMP_RESULT_LESS);
    RETURN_VAL_IF_TRUE(v2.isNull(), JsonCmpResult::JSON_CMP_RESULT_GREATER);
    RETURN_VAL_IF_TRUE(v1.type() != v2.type(), JsonCmpResult::JSON_CMP_RESULT_UNKNOWN);

#define RETURN_CMP_VALUE(a, b, type)                                                      \
    {                                                                                     \
        auto type##_v1 = a.as##type();                                                    \
        auto type##_v2 = b.as##type();                                                    \
        RETURN_VAL_IF_TRUE(type##_v1 == type##_v2, JsonCmpResult::JSON_CMP_RESULT_EQUAL); \
        RETURN_VAL_IF_TRUE(type##_v1 < type##_v2, JsonCmpResult::JSON_CMP_RESULT_LESS);   \
        return JsonCmpResult::JSON_CMP_RESULT_GREATER;                                    \
    }

    switch (v1.type())
    {
    case Json::ValueType::intValue:
        RETURN_CMP_VALUE(v1, v2, LargestInt);
    case Json::ValueType::uintValue:
        RETURN_CMP_VALUE(v1, v2, LargestUInt);
    case Json::ValueType::realValue:
    {
        auto double_v1 = v1.asDouble();
        auto double_v2 = v2.asDouble();
        RETURN_VAL_IF_TRUE(std::fabs(double_v1 - double_v2) < EPS, JsonCmpResult::JSON_CMP_RESULT_EQUAL);
        RETURN_VAL_IF_TRUE(double_v1 < double_v2, JsonCmpResult::JSON_CMP_RESULT_LESS);
        return JsonCmpResult::JSON_CMP_RESULT_GREATER;
    }
    case Json::ValueType::stringValue:
        RETURN_CMP_VALUE(v1, v2, String);
    case Json::ValueType::booleanValue:
        RETURN_CMP_VALUE(v1, v2, Bool);
    default:
        break;
    }
#undef RETURN_CMP_VALUE

    return JsonCmpResult::JSON_CMP_RESULT_UNKNOWN;
}

SSERuleRange::SSERuleRange(const Json::Value &min_value,
                           const Json::Value &max_value) : min_value_(min_value),
                                                           max_value_(max_value),
                                                           value_type_(Json::ValueType::nullValue)
{
    if (!min_value.isNull())
    {
        this->value_type_ = min_value.type();
    }
    else if (!max_value.isNull())
    {
        this->value_type_ = max_value.type();
    }
}

bool SSERuleRange::match(const Json::Value &value)
{
    RETURN_VAL_IF_TRUE(value.type() != this->value_type_, false);

    if (!this->min_value_.isNull())
    {
        auto result = this->json_value_cmp(this->min_value_, value);
        RETURN_VAL_IF_TRUE(result == JsonCmpResult::JSON_CMP_RESULT_UNKNOWN || result == JsonCmpResult::JSON_CMP_RESULT_GREATER, false);
    }

    if (!this->max_value_.isNull())
    {
        auto result = this->json_value_cmp(this->max_value_, value);
        RETURN_VAL_IF_TRUE(result == JsonCmpResult::JSON_CMP_RESULT_UNKNOWN || result == JsonCmpResult::JSON_CMP_RESULT_LESS, false);
    }

    return true;
}

SSERuleEnum::SSERuleEnum(const std::vector<Json::Value> &values) : enum_values_(values)
{
}

bool SSERuleEnum::match(const Json::Value &value)
{
    for (const auto &enum_value : this->enum_values_)
    {
        RETURN_VAL_IF_TRUE(this->json_value_cmp(enum_value, value) == JsonCmpResult::JSON_CMP_RESULT_EQUAL, true);
    }
    return false;
}

}  // namespace Kiran
