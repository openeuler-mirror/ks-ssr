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
#include "mask-widget.h"
#include <kiran-log/qt5-log-i.h>
#include <QGridLayout>
#include <QMouseEvent>
#include <QMovie>

namespace KS
{
MaskWidget::MaskWidget(QWidget *parent) : QWidget(parent), m_labLoading(nullptr)
{
    initUI();
}

void MaskWidget::setMaskVisible(bool visible)
{
    setVisible(visible);
}

bool MaskWidget::maskIsVisible()
{
    return isVisible();
}

MaskWidget::~MaskWidget()
{
}

void MaskWidget::initUI()
{
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setObjectName("maskWidget");

    auto gridLayout = new QGridLayout(this);

    m_labLoading = new QLabel(this);
    m_labLoading->setFixedSize(128, 128);
    auto movie = new QMovie(":/images/loading");
    m_labLoading->setMovie(movie);
    movie->start();

    gridLayout->addWidget(m_labLoading, 0, 0, Qt::AlignCenter);

    hide();
}
}  // namespace KS
