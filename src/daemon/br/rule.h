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

#pragma once

#include <QJsonValue>
#include "br-protocol.hxx"
#include "lib/base/base.h"

namespace KS
{
namespace BRDaemon
{
class Rule
{
public:
    // 规则类型
    virtual Protocol::RuleType getType() { return Protocol::RuleType::Value::NONE; };
    // 判断该值是否符合规则
    virtual bool match(const QJsonValue &value) { return false; };

    static QSharedPointer<Rule> create(const Protocol::Rule &rule);

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
    JsonCmpResult jsonValueCmp(const QJsonValue &v1, const QJsonValue &v2);
};

typedef QVector<QSharedPointer<Rule>> RuleVec;

class RuleRange : public Rule
{
public:
    // 如果min_value为空，则表示无限小，如果max_value为空，则表示无限大
    RuleRange(const QJsonValue &minValue,
              const QJsonValue &maxValue);
    virtual ~RuleRange(){};

    // 规则类型
    virtual Protocol::RuleType getType() override { return Protocol::RuleType::Value::RANGE; };
    // 判断该值是否符合规则
    virtual bool match(const QJsonValue &value) override;

private:
    QJsonValue m_minValue;
    QJsonValue m_maxValue;
    QJsonValue::Type m_valueType;
};

class RuleFixed : public RuleRange
{
public:
    // 如果min_value为空，则表示无限小，如果max_value为空，则表示无限大
    RuleFixed(const QJsonValue &value);
    virtual ~RuleFixed(){};

    // 规则类型
    virtual Protocol::RuleType getType() override { return Protocol::RuleType::Value::RANGE; };
};

class RuleEnum : public Rule
{
public:
    RuleEnum(const QVector<QJsonValue> &values);
    virtual ~RuleEnum(){};

    // 规则类型
    virtual Protocol::RuleType getType() override { return Protocol::RuleType::Value::ENUM; };
    // 判断该值是否符合规则
    virtual bool match(const QJsonValue &value) override;

private:
    QVector<QJsonValue> m_enumValues;
};

}  // namespace BRDaemon
}  // namespace KS
