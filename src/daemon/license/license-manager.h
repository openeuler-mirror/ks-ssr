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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */
#pragma once

#include <QDBusContext>
#include <QDBusObjectPath>
#include <QObject>
#include <QSharedPointer>

class LicenseAdaptor;

namespace KS
{
class LicenseProxy;

class LicenseManager : public QObject,
                       protected QDBusContext
{
    Q_OBJECT

public:
    static void globalInit(QObject *parent);
    static void globalDeinit();

    static LicenseManager *instance() { return m_instance; };

private:
    LicenseManager(QObject *parent = nullptr);
    virtual ~LicenseManager(){};

    void init();

public:  // PROPERTIES
    Q_PROPERTY(bool activated READ activated)
    bool activated() const;

public Q_SLOTS:  // METHODS
    void ActivateByActivationCode(const QString &activeCode);
    QString GetActivationCode();
    QString GetExpiredTime();
    QString GetMachineCode();

Q_SIGNALS:  // SIGNALS
    void LicenseChanged();

private:
    static LicenseManager *m_instance;

    QSharedPointer<LicenseProxy> m_licenseProxy;
    LicenseAdaptor *m_dbusAdaptor;
};
}  // namespace KS
