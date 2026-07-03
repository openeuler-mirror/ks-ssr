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
#include "progress-icon.h"
#include <QPainter>
#include <QTimer>

namespace KS
{
namespace BR
{
#define CIRCLE_SPEED_MS 50
static int angle;
ProgressIcon::ProgressIcon(QWidget *parent) : QWidget(parent)
{
    angle = 0;
    m_isFinishedProgress = false;
    initUI();
}

ProgressIcon::~ProgressIcon()
{
}

void ProgressIcon::finishedProgress(bool isFinish)
{
    m_isFinishedProgress = isFinish;
    if (!isFinish)
    {
        m_circlePixmapLabel->show();
        m_timer->start(CIRCLE_SPEED_MS);
    }
    else
    {
        m_circlePixmapLabel->hide();
        m_timer->stop();
    }
}

void ProgressIcon::paintEvent(QPaintEvent *event)
{
    // 绘制背景图片
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(m_isFinishedProgress ? ":/images/br-progress-finish" : ":/images/lightning"));
    // 抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);

    QWidget::paintEvent(event);
}

void ProgressIcon::initUI()
{
    // 无边框
    setWindowFlag(Qt::FramelessWindowHint);
    // 透明背景
    setAttribute(Qt::WA_TranslucentBackground, true);
    // 图片宽度
    setFixedSize(58, 58);
    m_circlePixmap.load(":/images/circle");
    m_circlePixmap.scaled(58, 58);
    m_circlePixmapLabel = new QLabel(this);
    m_circlePixmapLabel->setPixmap(m_circlePixmap);

    // 旋转图片定时器
    m_timer = new QTimer(this);

    connect(m_timer, &QTimer::timeout, this, &ProgressIcon::circlePixmap);
}

void ProgressIcon::circlePixmap()
{
    if (angle >= 360)
    {
        angle = 0;
    }
    angle += 10;
    angle %= 360;

    QMatrix matrix;
    matrix.rotate(angle);
    // 旋转绘制坐标
    QPixmap tmpCirclePixmap(size());
    tmpCirclePixmap.fill(Qt::transparent);
    QPainter painter(&tmpCirclePixmap);
    // 设置旋转中心
    painter.translate(width() / 2, height() / 2);
    painter.rotate(angle);
    // 原点复位
    painter.translate(-width() / 2, -height() / 2);
    painter.drawPixmap(0, 0, width(), height(), m_circlePixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    m_circlePixmapLabel->setPixmap(tmpCirclePixmap);
}
}  // namespace BR
}  // namespace KS
