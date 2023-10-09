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

#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include "include/ssr-marcos.h"
#include "ui_qrcode-dialog.h"

namespace KS
{
namespace Activation
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
    setFixedSize(359, 319);
}

QPixmap QRCodeDialog::createQRcodePixmap(const QString &text)
{
    QPixmap QRPixmap;
    m_qrcode = QRcode_encodeString(text.toStdString().c_str(), 2, QR_ECLEVEL_Q, QR_MODE_8, 1);

    int qrcodeWidth = m_qrcode->width > 0 ? m_qrcode->width : 1;
    double scaledWidth = (double)ui->m_qrcodeImage->width() / (double)qrcodeWidth;
    double scaledHeight = (double)ui->m_qrcodeImage->height() / (double)qrcodeWidth;
    QImage image = QImage(ui->m_qrcodeImage->width(), ui->m_qrcodeImage->height(), QImage::Format_ARGB32);

    QPainter painter(&image);
    QColor background(Qt::white);
    painter.setBrush(background);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, ui->m_qrcodeImage->width(), ui->m_qrcodeImage->height());
    QColor foreground(Qt::black);
    painter.setBrush(foreground);

    for (qint32 y = 0; y < qrcodeWidth; y++)
    {
        for (qint32 x = 0; x < qrcodeWidth; x++)
        {
            unsigned char b = m_qrcode->data[y * qrcodeWidth + x];
            if (b & 0x01)
            {
                QRectF r(x * scaledWidth, y * scaledHeight, scaledWidth, scaledHeight);
                painter.drawRects(&r, 1);
            }
        }
    }

    QRPixmap = QPixmap::fromImage(image);
    return QRPixmap;
}

void QRCodeDialog::setText(const QString &text)
{
    RETURN_IF_TRUE(text.isEmpty());

    ui->m_qrcodeImage->setPixmap(createQRcodePixmap(text));
}

void QRCodeDialog::setSummary(const QString &text)
{
    ui->m_summary->setText(text);
}

QRCodeDialog::~QRCodeDialog()
{
    delete ui;
}
}  // namespace Activation
}  // namespace KS
