/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#pragma once

#include <br-plugin-i.h>
#include <QMap>
#include <QSharedPointer>
#include "lib/base/base.h"

namespace KS
{
namespace BR
{
class ReinforcementBash : public BRReinforcementInterface
{
public:
    ReinforcementBash(const QString &bashFilePath);
    virtual ~ReinforcementBash(){};

    virtual bool init();
    virtual bool isInit();
    virtual bool get(QString &args, QString &error);
    virtual bool set(const QString &args, QString &error);
    virtual bool backup(QString &args, QString &error);
    virtual bool rollback(const QString &args, QString &error);

private:
    QString m_bashFilePath;
};

class PluginBash : public BRPluginInterface
{
public:
    PluginBash(const QString &bashRootDir);
    virtual ~PluginBash(){};

    virtual void activate() override;
    virtual void deactivate() override;

    virtual QSharedPointer<BRReinforcementInterface> getReinforcement(const QString &name) override
    {
        return MapHelper::getValue(this->m_reinforcements, name);
    };

private:
    QString m_bashRootDir;
    QString m_reinforcementConfigPath;
    QMap<QString, QSharedPointer<BRReinforcementInterface>> m_reinforcements;
};
}  // namespace BR
}  // namespace KS
