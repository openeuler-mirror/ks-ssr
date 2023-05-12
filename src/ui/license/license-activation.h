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
#include "src/ui/common/titlebar-window.h"

namespace Ui
{
class LicenseActivation;
}

namespace KS
{
class LicenseDBus;
class QRCodeDialog;
class LicenseActivation : public TitlebarWindow
{
    Q_OBJECT

public:
    LicenseActivation(QWidget* parent = nullptr);
    virtual ~LicenseActivation();
    /**
     * @brief update:将授权信息设置在ui界面中
     */
    void update();

protected:
    void closeEvent(QCloseEvent* event);

private:
    void initUI();

private slots:
    void activate();
    void popupQrencode();

signals:
    void closed();

private:
    Ui::LicenseActivation* m_ui;
    QSharedPointer<LicenseDBus> m_licenseDBus;
    QRCodeDialog* m_qrcodeDialog;
};
}  // namespace KS
