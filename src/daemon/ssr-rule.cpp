/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-rule.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "src/daemon/ssr-rule.h"

namespace Kiran
{
using namespace RS;

std::shared_ptr<SSRRule> SSRRule::create(const Json::Value &rule)
{
    RETURN_VAL_IF_TRUE(!rule[SSR_JSON_BODY_RULES_TYPE].isInt(), nullptr);
    auto type = rule[SSR_JSON_BODY_RULES_TYPE].asInt();

    switch (type)
    {
        // TODO: 实现FIXED/ENUM
    case SSRRuleType::Value::RANGE:
    {
        RETURN_VAL_IF_TRUE(!rule.isMember(SSR_JSON_BODY_RULES_MIN_VALUE) && !rule.isMember(SSR_JSON_BODY_RULES_MAX_VALUE), nullptr);
        if (rule.isMember(SSR_JSON_BODY_RULES_MIN_VALUE) &&
            rule.isMember(SSR_JSON_BODY_RULES_MAX_VALUE) &&
            rule[SSR_JSON_BODY_RULES_MIN_VALUE].type() != rule[SSR_JSON_BODY_RULES_MAX_VALUE].type())
        {
            KLOG_WARNING("The type of min_value and max_value is not equal.");
            return nullptr;
        }
        return std::make_shared<SSRRuleRange>(rule[SSR_JSON_BODY_RULES_MIN_VALUE], rule[SSR_JSON_BODY_RULES_MAX_VALUE]);
    }
    default:
        break;
    }
    return nullptr;
}

std::shared_ptr<SSRRule> SSRRule::create(const SSRRSRule &rule)
{
    switch (rule.type())
    {
    case SSRRuleType::Value::FIXED:
    {
        RETURN_VAL_IF_FALSE(rule.value_fixed().present(), nullptr);
        auto value = StrUtils::str2json(rule.value_fixed().get());
        return std::make_shared<SSRRuleFixed>(value);
    }
    case SSRRuleType::Value::RANGE:
    {
        RETURN_VAL_IF_FALSE(rule.value_range().present(), nullptr);
        auto &value_range = rule.value_range().get();
        auto min_value = StrUtils::str2json(value_range.min_value());
        auto max_value = StrUtils::str2json(value_range.max_value());
        return std::make_shared<SSRRuleRange>(min_value, max_value);
    }
    case SSRRuleType::Value::ENUM:
    {
        RETURN_VAL_IF_FALSE(rule.value_enum().present(), nullptr);
        auto &value_enum = rule.value_enum().get();
        std::vector<Json::Value> values;
        for (const auto &enum_value : value_enum.values())
        {
            auto value = StrUtils::str2json(enum_value);
            values.push_back(value);
        }
        return std::make_shared<SSRRuleEnum>(values);
        break;
    }
    default:
        break;
    }
    return nullptr;
}

SSRRule::JsonCmpResult SSRRule::json_value_cmp(const Json::Value &v1, const Json::Value &v2)
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

SSRRuleRange::SSRRuleRange(const Json::Value &min_value,
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

bool SSRRuleRange::match(const Json::Value &value)
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

SSRRuleFixed::SSRRuleFixed(const Json::Value &value) : SSRRuleRange(value, value)
{
}

SSRRuleEnum::SSRRuleEnum(const std::vector<Json::Value> &values) : enum_values_(values)
{
}

bool SSRRuleEnum::match(const Json::Value &value)
{
    for (const auto &enum_value : this->enum_values_)
    {
        RETURN_VAL_IF_TRUE(this->json_value_cmp(enum_value, value) == JsonCmpResult::JSON_CMP_RESULT_EQUAL, true);
    }
    return false;
}

}  // namespace Kiran
