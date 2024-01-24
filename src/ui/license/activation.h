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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#pragma once

#include "src/ui/common/window/titlebar-window.h"

namespace Ui
{
class Activation;
}

namespace KS
{
class LicenseProxy;

namespace Activation
{
class QRCodeDialog;
class Activation : public TitlebarWindow
{
    Q_OBJECT

public:
    Activation(QWidget* parent = nullptr);
    virtual ~Activation();
    /**
     * @brief update:将授权信息设置在ui界面中
     */
    void update();

private:
    void initUI();
    void popupQRcode(const QString& QRcode, const QString& title);

private slots:
    void activate();
    void handleQrcode();
signals:
    // 发送错误或激活成功消息
    void activated(const QString& message);

private:
    Ui::Activation* m_ui;
    QSharedPointer<KS::LicenseProxy> m_licenseProxy;
    QRCodeDialog* m_qrcodeDialog;
};
}  // namespace Activation
}  // namespace KS