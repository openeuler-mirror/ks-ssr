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
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#pragma once

#include <QList>
#include <QObject>
#include <QSharedPointer>
#include "src/daemon/dm/configuration.h"

namespace KS
{
namespace DM
{
struct DeviceRule
{
public:
    DeviceRule() = default;
    QString idVendor;
    QString idProduct;
    bool read;
    bool write;
    bool execute;
    int interfaceType;
};

class UdevRuleManager : public QObject
{
    Q_OBJECT
public:
    static UdevRuleManager *instance();

private:
    explicit UdevRuleManager(QObject *parent = nullptr);

private Q_SLOTS:
    void handleDevSettingChanged();

private:
    void init();
    void updateUdevRules();
    void updateToFile();
    void saveToFile(const QStringList &lines);
    QSharedPointer<DeviceRule> find(const QString idVendor,
                                    const QString idProduct);
    QString getUdevModeValue(QSharedPointer<DeviceRule> rule);
    QString rulleObj2Str(QSharedPointer<DeviceRule> rule);

private:
    QList<QSharedPointer<DeviceRule>> m_rules;
    Configuration *m_deviceConfig;
};
}  // namespace DM
}  // namespace KS
