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
#include "qrcode-dialog.h"
#include <QIcon>

#include "ui_qrcode-dialog.h"

namespace KS
{
QRCodeDialog::QRCodeDialog(QWidget *parent) : QWidget(parent),
                                              ui(new Ui::QRCodeDialog)
{
    ui->setupUi(this);
    iniUI();
}

void QRCodeDialog::iniUI()
{
    setWindowFlags(Qt::X11BypassWindowManagerHint);
    setWindowTitle(QRCodeDialog::tr("QRcode of Machine and Activation Code"));
    setWindowIcon(QIcon(":/images/icons/ksl-os-24.png"));
    setAttribute(Qt::WA_TransparentForMouseEvents);
    this->setMaximumSize(230, 230);
    this->setMinimumSize(230, 230);
    ui->qrcode_layout->setMargin(10);
}

void QRCodeDialog::setQRCode(const QString &text, bool isMachineCode)
{
    qrcode = new QRCode(this, text, 160, 160);
    qrcode->setMinimumSize(160, 160);
    qrcode->setMaximumSize(160, 160);
    ui->qrcode_layout->addWidget(qrcode, Qt::AlignHCenter);
    ui->qrcode_layout->setAlignment(qrcode, Qt::AlignHCenter);
    ui->qrcode_layout->setAlignment(qrcode, Qt::AlignVCenter);

    if (!isMachineCode)
    {
        ui->label_qrcode_text->setText(tr("Scan QR code to get activation code"));
    }
}

QRCodeDialog::~QRCodeDialog()
{
    delete ui;
}
}  // namespace KS
