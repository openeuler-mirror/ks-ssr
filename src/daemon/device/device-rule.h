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
#include <QSettings>
#include <QSharedPointer>

namespace KS
{
struct Rule
{
public:
    Rule() = default;
    QString uid;
    QString id;
    QString name;
    QString idVendor;
    QString idProduct;
    bool read;
    bool write;
    bool execute;
    bool enable;
    int type;
    int interfaceType;
};

class DeviceRule : public QObject
{
    Q_OBJECT

private:
    explicit DeviceRule(QObject *parent = nullptr);

public:
    static DeviceRule *instance();
    void addRule(const Rule &rule);
    QSharedPointer<Rule> getRule(const QString &uid);

private:
    void init();
    QStringList toUdevRules();
    QString rule2UdevRule(QSharedPointer<Rule> rule);
    QString getUdevModeValue(QSharedPointer<Rule> rule);
    void updateUdevFile();
    bool groupExisted(const QString group);

private:
    QSettings *m_settings;
};
}  // namespace KS
