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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qrencode.h>
#include <QLabel>
#include <QPaintEvent>
#include <QWidget>

class QRCode : public QLabel
{
    Q_OBJECT

public:
    explicit QRCode(QWidget *parent, const QString &text, int width, int height);
    ~QRCode();

    const QString &getData();
    void setData(const QString &data_);

    void createQRcode(int width, int height);

private:
    QString m_data;
    QRcode *m_qrcode;
};

#endif  // MAINWINDOW_H
