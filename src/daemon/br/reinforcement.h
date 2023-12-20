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

#include "br-protocol.hxx"
#include "lib/base/base.h"
#include "rule.h"

namespace KS
{
namespace BRDaemon
{
class Reinforcement
{
public:
    Reinforcement() = delete;
    Reinforcement(const QString &plugin_id,
                  const Protocol::Reinforcement &rs);
    virtual ~Reinforcement(){};

    QString getName()
    {
        return QString::fromStdString(this->config_.name());
    };
    QString getPluginName()
    {
        return this->plugin_id_;
    };
    QString getCategoryName();
    QString getLabel();

    void setRs(const Protocol::Reinforcement &rs);
    const Protocol::Reinforcement &getRs()
    {
        return this->config_;
    };

    // 判断与规则是否匹配
    bool matchRules(const QJsonObject &values);

private:
    void reload();
    void updateRules();

private:
    // 加固项所属插件ID
    QString plugin_id_;
    // 加固项的加固标准
    Protocol::Reinforcement config_;
    // 标准的判断规则
    QMap<QString, QSharedPointer<Rule>> rules_;
};

typedef QVector<QSharedPointer<Reinforcement>> BRReinforcementVec;

}  // namespace BRDaemon
}  // namespace KS
