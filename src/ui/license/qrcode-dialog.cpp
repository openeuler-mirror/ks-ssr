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
QRCodeDialog::QRCodeDialog(QWidget *parent) : TitlebarWindow(parent),
                                              ui(new Ui::QRCodeDialog)
{
    ui->setupUi(getWindowContentWidget());
    iniUI();
}

void QRCodeDialog::iniUI()
{
    setWindowModality(Qt::WindowModal);
    setTitle(tr("QR code"));
    setIcon(QIcon(":/images/logo"));
    setButtonHints(TitlebarCloseButtonHint);
    setFixedSize(350, 300);
    ui->m_layout->setMargin(10);
}

void QRCodeDialog::setQRCode(const QString &text)
{
    auto qrcode = new QRCode(this, text, 160, 160);
    qrcode->setMinimumSize(160, 160);
    qrcode->setMaximumSize(160, 160);
    ui->m_layout->addWidget(qrcode, Qt::AlignHCenter);
    ui->m_layout->setAlignment(qrcode, Qt::AlignHCenter);
    ui->m_layout->setAlignment(qrcode, Qt::AlignVCenter);
}

QRCodeDialog::~QRCodeDialog()
{
    delete ui;
}
}  // namespace KS
