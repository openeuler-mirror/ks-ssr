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
struct DeviceRule
{
public:
    DeviceRule() = default;
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

class DeviceRuleManager : public QObject
{
    Q_OBJECT

private:
    explicit DeviceRuleManager(QObject *parent = nullptr);

public:
    static DeviceRuleManager *instance();
    void addRule(const DeviceRule &rule);
    QSharedPointer<DeviceRule> getRule(const QString &uid);

    bool isIFCEnable(int type);
    bool setIFCEnable(int type,
                      bool enable);

private:
    void init();
    QStringList getUdevRules();
    QString rule2UdevRule(QSharedPointer<DeviceRule> rule);
    void updateUdevFile();
    void updateIFCUdevFile();
    void saveToFile(const QStringList &rules, 
                    const QString &filename);

    QStringList getIFCUdevRules();
    QString getIFCUdevRule(int type);
    QString getUdevModeValue(QSharedPointer<DeviceRule> rule);

    bool groupExisted(const QString group);

private:
    QSettings *m_settings;
    QSettings *m_ifcSettings;
};
}  // namespace KS
