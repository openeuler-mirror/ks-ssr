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

#include "round-progressbar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPen>

namespace KS
{
namespace BR
{
RoundProgressBar::RoundProgressBar(const QString &name,
                                   int total,
                                   int conform,
                                   int inconform,
                                   QWidget *parent)
    : QWidget(parent),
      m_name(name),
      m_total(total),
      m_conform(conform),
      m_inconform(inconform)
{
    initUI();
}

QSize RoundProgressBar::sizeHint() const
{
    return QSize(379, 250);
}

void RoundProgressBar::paintEvent(QPaintEvent *)
{
    int width = 160;
    int height = 160;
    int size = qMin(width, height);

    int m_startAngle = 315;
    const QRectF drawingRectF(8.0, 8.0, size, size);

    QPen pen;
    pen.setWidth(8);
    pen.setCapStyle(Qt::RoundCap);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(100, 0);

    // 背景颜色
    painter.save();
    pen.setBrush(QColor(m_total == 0 ? "#f2f2f2" : "#ed6262"));
    painter.setPen(pen);
    painter.drawArc(drawingRectF, 0, 360 * 16);
    painter.restore();

    if (m_percent != 0)
    {
        int span = 3.6 * m_percent;

        painter.save();
        pen.setBrush(QColor("#f2f2f2"));
        painter.setPen(pen);
        painter.drawArc(drawingRectF, (m_startAngle - 0) * 16, (span + 0) * 16);
        painter.restore();

        painter.save();
        pen.setWidth(8);
        pen.setBrush(QColor("#f2f2f2"));
        painter.setPen(pen);
        painter.drawArc(drawingRectF, m_startAngle * 16, span * 16);
        painter.restore();
    }
}

void RoundProgressBar::initUI()
{
    if (m_total == 0)
        m_percent = 0;
    else
        m_percent = m_conform / (float)m_total * 100;

    sizeHint();
    setFixedSize(379, 250);

    m_percentLabel = new QLabel(this);
    m_percentLabel->setObjectName("m_percentLabel");
    m_percentLabel->setFixedSize(379, 40);
    m_percentLabel->setAlignment(Qt::AlignCenter);
    m_percentLabel->move(0, 70);
    m_percentLabel->setText(QString("%1%").arg(QString::number(m_percent, 'f', 2)));
    // TODO 这里在qss中设置会覆盖掉paintEvent绘制的颜色，需要考虑优化
    m_percentLabel->setStyleSheet("QLabel{background-color: transparent;}");

    m_nameLabel = new QLabel(this);
    m_nameLabel->setObjectName("m_nameLabel");
    m_nameLabel->setFixedSize(379, 30);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->move(0, 180);
    m_nameLabel->setText(m_name);

    m_noteLabel = new QLabel(this);
    m_noteLabel->setObjectName("m_noteLabel");
    m_noteLabel->setFixedSize(379, 20);
    m_noteLabel->setAlignment(Qt::AlignCenter);
    m_noteLabel->move(0, 220);
    m_noteLabel->setText(QString(tr("Total: %1 conform: %2 inconform: %3")).arg(m_total).arg(m_conform).arg(m_inconform));
}
}  // namespace BR
}  // namespace KS
