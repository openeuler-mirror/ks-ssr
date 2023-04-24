/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#pragma once

#include <QObject>

namespace KS
{
class DeviceRule : public QObject
{
    Q_OBJECT

private:
    explicit DeviceRule(QObject *parent = nullptr);

public:
    static DeviceRule *instance();
    bool addRule(const QString &rule);
    bool updateRule(const QString &rule, const QString &newRule);
    QString findRule(const QString &str);

private:
    void init();
    bool updateRulesToFile();

private:
    QStringList m_ruleList;
};
}  // namespace KS
