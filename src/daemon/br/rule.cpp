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

#include "rule.h"

namespace KS
{
namespace BRDaemon
{
QSharedPointer<Rule> Rule::create(const Protocol::Rule &rule)
{
    switch (rule.type())
    {
    case Protocol::RuleType::Value::FIXED:
    {
        RETURN_VAL_IF_FALSE(rule.value_fixed().present(), QSharedPointer<Rule>());
        // 如果 xsd 的定义时能使用 QString 的话应该可以少很多临时变量
        auto value = StrUtils::str2jsonValue(rule.value_fixed().get());
        return QSharedPointer<RuleFixed>::create(value);
    }
    case Protocol::RuleType::Value::RANGE:
    {
        RETURN_VAL_IF_FALSE(rule.value_range().present(), QSharedPointer<Rule>());
        auto &value_range = rule.value_range().get();
        QJsonValue min_value;
        QJsonValue max_value;
        if (value_range.min_value().present())
        {
            min_value = StrUtils::str2jsonValue(value_range.min_value().get());
        }
        if (value_range.max_value().present())
        {
            max_value = StrUtils::str2jsonValue(value_range.max_value().get());
        }
        return QSharedPointer<RuleRange>::create(min_value, max_value);
    }
    case Protocol::RuleType::Value::ENUM:
    {
        RETURN_VAL_IF_FALSE(rule.value_enum().present(), QSharedPointer<Rule>());
        auto &value_enum = rule.value_enum().get();
        QVector<QJsonValue> values;
        for (const auto &enum_value : value_enum.values())
        {
            auto value = StrUtils::str2jsonValue(enum_value);
            values.push_back(value);
        }
        return QSharedPointer<RuleEnum>::create(values);
    }
    default:
        break;
    }
    return QSharedPointer<Rule>();
}

Rule::JsonCmpResult Rule::jsonValueCmp(const QJsonValue &v1, const QJsonValue &v2)
{
    RETURN_VAL_IF_TRUE(v1.isNull() && v2.isNull(), JsonCmpResult::JSON_CMP_RESULT_EQUAL);
    RETURN_VAL_IF_TRUE(v1.isNull(), JsonCmpResult::JSON_CMP_RESULT_LESS);
    RETURN_VAL_IF_TRUE(v2.isNull(), JsonCmpResult::JSON_CMP_RESULT_GREATER);
    RETURN_VAL_IF_TRUE(v1.type() != v2.type(), JsonCmpResult::JSON_CMP_RESULT_UNKNOWN);

#define RETURN_CMP_VALUE(a, b, type)                                                      \
    {                                                                                     \
        auto type##_v1 = a.to##type();                                                    \
        auto type##_v2 = b.to##type();                                                    \
        RETURN_VAL_IF_TRUE(type##_v1 == type##_v2, JsonCmpResult::JSON_CMP_RESULT_EQUAL); \
        RETURN_VAL_IF_TRUE(type##_v1 < type##_v2, JsonCmpResult::JSON_CMP_RESULT_LESS);   \
        return JsonCmpResult::JSON_CMP_RESULT_GREATER;                                    \
    }

    switch (v1.type())
    {
    case QJsonValue::Type::Double:
        // QJsonValue 无法直接区分 Double 和 Int，但是它在 toInt 尝试获取值时，如果此值不是个整数，则会返回默认值，
        // 在这里将默认值设置为 -777777 ，减少默认值冲突的可能性
        if (-777777 != v1.toDouble(-777777))
        {
            {
                auto double_v1 = v1.toDouble();
                auto double_v2 = v2.toDouble();
                RETURN_VAL_IF_TRUE(std::fabs(double_v1 - double_v2) < EPS, JsonCmpResult::JSON_CMP_RESULT_EQUAL);
                RETURN_VAL_IF_TRUE(double_v1 < double_v2, JsonCmpResult::JSON_CMP_RESULT_LESS);
                return JsonCmpResult::JSON_CMP_RESULT_GREATER;
            }
        }
        RETURN_CMP_VALUE(v1, v2, Int);
        break;
    case QJsonValue::Type::String:
        RETURN_CMP_VALUE(v1, v2, String);
        break;
    case QJsonValue::Type::Bool:
        RETURN_CMP_VALUE(v1, v2, Bool);
        break;
    default:
        break;
    }
#undef RETURN_CMP_VALUE

    return JsonCmpResult::JSON_CMP_RESULT_UNKNOWN;
}

RuleRange::RuleRange(const QJsonValue &min_value,
                     const QJsonValue &max_value)
    : min_value_(min_value),
      max_value_(max_value),
      value_type_(QJsonValue::Type::Undefined)
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

bool RuleRange::match(const QJsonValue &value)
{
    // 如果最大值和最小值都为空，则表示不限制
    RETURN_VAL_IF_TRUE(this->min_value_.isNull() && this->max_value_.isNull(), true);
    RETURN_VAL_IF_TRUE(value.type() != this->value_type_, false);

    if (!this->min_value_.isNull())
    {
        auto result = this->jsonValueCmp(this->min_value_, value);
        RETURN_VAL_IF_TRUE(result == JsonCmpResult::JSON_CMP_RESULT_UNKNOWN || result == JsonCmpResult::JSON_CMP_RESULT_GREATER, false);
    }

    if (!this->max_value_.isNull())
    {
        auto result = this->jsonValueCmp(this->max_value_, value);
        RETURN_VAL_IF_TRUE(result == JsonCmpResult::JSON_CMP_RESULT_UNKNOWN || result == JsonCmpResult::JSON_CMP_RESULT_LESS, false);
    }

    return true;
}

RuleFixed::RuleFixed(const QJsonValue &value)
    : RuleRange(value, value)
{
}

RuleEnum::RuleEnum(const QVector<QJsonValue> &values)
    : enum_values_(values)
{
}

bool RuleEnum::match(const QJsonValue &value)
{
    for (const auto &enum_value : this->enum_values_)
    {
        RETURN_VAL_IF_TRUE(this->jsonValueCmp(enum_value, value) == JsonCmpResult::JSON_CMP_RESULT_EQUAL, true);
    }
    return false;
}
}  // namespace BRDaemon
}  // namespace KS
