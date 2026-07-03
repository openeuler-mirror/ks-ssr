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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#pragma once

#include <kylin-license/license-i.h>
#include <QObject>
#include <QString>
#include <QWidget>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QtDBus>

#define LICENSE_OBJECT_NAME "KSSC"
#define TIMEOUT_MS 5000
#define METHOD_GET_LICENSE "GetLicense"
#define METHOD_GET_LICENSE_OBJECT "GetLicenseObject"
#define METHOD_ACTIVATE_BY_ACTIVATION_CODE "ActivateByActivationCode"
#define METHOD_DEACTIVE_LICENSE "DeactiveLicense"
#define SIGNAL_LICENSE_CHANGED "LicenseChanged"

namespace KS
{
class LicenseDBus : public QObject
{
    Q_OBJECT
public:
    LicenseDBus(QObject* parent = 0);
    virtual ~LicenseDBus();

public:
    bool creatObjectName(const QString& objectName);
    QString getLicense();
    bool activateByActivationCode(const QString& activation_Code, QString& errorMsg);

signals:
    void licenseChanged(bool);

private slots:
    void licenseChange(bool);
};
}  // namespace KS
