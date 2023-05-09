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
#include <QDialog>
#include "src/ui/common/titlebar-window.h"

namespace Ui
{
class Activation;
}

namespace KS
{
struct LicenseInfo
{
    LicenseInfo() : activationCode(""),
                    machineCode(""),
                    activationStatus(LicenseActivationStatus::LAS_LAST),
                    expiredTime(0) {}

    QString activationCode;
    QString machineCode;
    int activationStatus;
    time_t expiredTime;
};

class LicenseUtils;
class QRCodeDialog;
class Activation : public TitlebarWindow
{
    Q_OBJECT

public:
    Activation(QWidget* parent = nullptr);
    virtual ~Activation();
    bool isActivate();
    void updateLicenseInfo();

protected:
    void closeEvent(QCloseEvent* event);

private:
    void initUI();
    void getLicenseInfo();
    void setLicenseInfo();

private slots:
    void activate();
    void popupQrencode();
    void getLicense(bool isChanged);

signals:
    void activated(bool);
    void closed();

private:
    Ui::Activation* m_ui;
    LicenseUtils* m_licenseUtils;
    LicenseInfo m_licenseInfo;
    QRCodeDialog* m_qrcodeDialog;
};
}  // namespace KS
