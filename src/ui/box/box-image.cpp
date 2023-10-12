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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */
#include "box-image.h"
#include <QPainter>

namespace KS
{
namespace Box
{
BoxImage::BoxImage(QWidget *parent, const QString &imagePath) : QWidget(parent)
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFixedSize(70, 70);

    m_image = new QImage(imagePath);
    m_drawArea = QRect(0, 0, width(), width());

    move(m_image->width() * 5, m_image->height() * 5);
}

void BoxImage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/images/box-big"));
    // 抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);
    qreal x = m_drawArea.center().x() + width() / 2 - m_image->width() + 4;
    qreal y = m_drawArea.center().y() + height() / 2 - m_image->height() + 4;
    painter.drawImage(x, y, *m_image);

    QWidget::paintEvent(event);
}
}  // namespace Box
}  // namespace KS