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
class LicenseActivation;
}

namespace KS
{
struct LicenseInfo
{
    LicenseInfo() : activationStatus(LicenseActivationStatus::LAS_LAST),
                    expiredTime(0) {}

    QString activationCode;
    QString machineCode;
    int activationStatus;
    time_t expiredTime;
};

class LicenseDBus;
class QRCodeDialog;
class LicenseActivation : public TitlebarWindow
{
    Q_OBJECT

public:
    LicenseActivation(QWidget* parent = nullptr);
    virtual ~LicenseActivation();
    /**
     * @brief isActivate:判断是否已授权
     * @return true： 已授权
     *         false：未授权/授权已过期
     */
    bool isActivate();

    /**
     * @brief update:通过dbus获取授权信息，并将数据更新在ui界面中
     */
    void update();

protected:
    void closeEvent(QCloseEvent* event);

private:
    void initUI();
    void getLicenseInfo();

private slots:
    void activate();
    void popupQrencode();

    /**
     * @brief setLicense:授权信息变化的槽函数，用于重新获取授权信息并设置在ui界面中，发送应用是否授权信号
     */
    void setLicense();

signals:
    void activated(bool);
    void closed();

private:
    Ui::LicenseActivation* m_ui;
    LicenseDBus* m_licenseDbus;
    LicenseInfo m_licenseInfo;
    QRCodeDialog* m_qrcodeDialog;
};
}  // namespace KS
