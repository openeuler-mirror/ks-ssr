/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-cpanel-system is licensed under Mulan PSL v2.
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

#include <qrencode.h>
#include "src/ui/common/window/titlebar-window.h"

namespace Ui
{
class QRCodeDialog;
}

namespace KS
{
namespace Activation
{
class QRCodeDialog : public TitlebarWindow
{
    Q_OBJECT

public:
    explicit QRCodeDialog(QWidget *parent = 0);
    ~QRCodeDialog();

private:
    void iniUI();
    QPixmap createQRcodePixmap(const QString &text);

public:
    void setText(const QString &text);
    void setSummary(const QString &text);

private:
    Ui::QRCodeDialog *ui;
    QRcode *m_qrcode;
};
}  // namespace Activation
}  // namespace KS
