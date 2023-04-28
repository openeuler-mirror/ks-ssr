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
#include "qrcode.h"
#include <kiran-log/qt5-log-i.h>
#include <QPaintEvent>
#include <QPainter>

QRCode::QRCode(QWidget *parent, const QString &text, int width, int height) : QLabel(parent),
                                                                              m_data(text)
{
    createQRcode(width, height);
}

QRCode::~QRCode()
{
    if (m_qrcode)
    {
        QRcode_free(m_qrcode);
    }
}

const QString &QRCode::getData()
{
    return m_data;
}

void QRCode::setData(const QString &data_)
{
    m_data = data_;
}

void QRCode::createQRcode(int width, int height)
{
    QPixmap QRPixmap;
    m_qrcode = QRcode_encodeString(m_data.toStdString().c_str(), 2, QR_ECLEVEL_Q, QR_MODE_8, 1);

    int qrcodeWidth = m_qrcode->width > 0 ? m_qrcode->width : 1;
    double scaledWidth = (double)width / (double)qrcodeWidth;
    double scaledHeight = (double)height / (double)qrcodeWidth;
    QImage image = QImage(width, height, QImage::Format_ARGB32);

    QPainter painter(&image);
    QColor background(Qt::white);
    painter.setBrush(background);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, width, height);
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
    setPixmap(QRPixmap);
}
