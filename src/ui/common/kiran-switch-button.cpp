/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiranwidgets-qt5 is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

#include "kiran-switch-button.h"
#include "kiran-switch-button-private.h"

#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
namespace KS
{
#define IndicatorHeight 24
#define ItemSpacing 8
#define IndicatorWidth 54

KiranSwitchButton::KiranSwitchButton(QWidget *parent)
    : QAbstractButton(parent),
      d_ptr(new KiranSwitchButtonPrivate(this))
{
    setCheckable(true);
    setAccessibleName("KiranSwitchButton");
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

KiranSwitchButton::~KiranSwitchButton()
{
    delete d_ptr;
}

bool KiranSwitchButton::event(QEvent *e)
{
    return QAbstractButton::event(e);
}

void KiranSwitchButton::paintEvent(QPaintEvent *e)
{
    QStyleOptionButton option;
    initStyleOption(&option);

    bool enable = option.state & QStyle::State_Enabled;

    //    StylePalette *kiranPalette = StylePalette::instance();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect indicatorRect, indicatorCircularRect, textRect;
    d_ptr->doLayout(indicatorRect, indicatorCircularRect, textRect);

    QColor indicatorBackgroundColor, indicatorCircularColor, textColor;
    indicatorBackgroundColor = enable ? QColor(0, 162, 255) : QColor(70, 70, 70);  //window()->palette().window().color();
    indicatorCircularColor = "white";

    QPainterPath indicatorPath;
    indicatorPath.addRoundedRect(indicatorRect, indicatorRect.height() / 2, indicatorRect.height() / 2);
    painter.fillPath(indicatorPath, QBrush(indicatorBackgroundColor));

    QPainterPath indicatorCircularPath;
    indicatorCircularPath.addRoundedRect(indicatorCircularRect, indicatorCircularRect.height() / 2, indicatorCircularRect.height() / 2);
    painter.fillPath(indicatorCircularPath, indicatorCircularColor);

    QFont f = font();
    QFontMetrics fm = option.fontMetrics;
    painter.drawText(textRect, Qt::AlignCenter, option.text);
}

QSize KiranSwitchButton::sizeHint() const
{
    QStyleOptionButton optionButton;
    initStyleOption(&optionButton);

    QSize size(IndicatorWidth, IndicatorHeight);

    QString str = text();
    if (!str.isEmpty())
    {
        QFontMetrics metrics = fontMetrics();
        size.setHeight(qMax(metrics.size(Qt::TextHideMnemonic, str).height(), IndicatorHeight));
        size += 2 * QSize(3, 3);
        size.rwidth() += ItemSpacing;
    }

    return size;
}

void KiranSwitchButton::initStyleOption(QStyleOptionButton *option) const
{
    option->init(this);
    option->initFrom(this);

    if (isChecked())
    {
        option->state |= QStyle::State_On;
    }
    else
    {
        option->state |= QStyle::State_Off;
    }

    option->text = text();
}

QSize KiranSwitchButton::minimumSizeHint() const
{
    return sizeHint();
}

void KiranSwitchButtonPrivate::doLayout(QRect &indicatorRect, QRect &indicatorCircularRect, QRect &textRect)
{
    QStyleOptionButton option;
    q_ptr->initStyleOption(&option);

    QRect rect = option.rect;

    //指示器背景
    QSize indicatorSize(IndicatorWidth, IndicatorHeight);
    QPoint indicatorTopLeft(0, rect.top() + (rect.height() - indicatorSize.height()) / 2);
    indicatorRect = QRect(indicatorTopLeft, indicatorSize);

    //指示器之中圆形开关
    QSize indicatorCircularSize(indicatorSize.height() - 8, indicatorSize.height() - 8);
    QPoint indicatorCircularTopLeft(indicatorRect.left() + 2, indicatorRect.top() + (indicatorRect.height() - indicatorCircularSize.height()) / 2);
    indicatorCircularRect = QRect(indicatorCircularTopLeft, indicatorCircularSize);
    if (option.state & QStyle::State_On)
    {
        indicatorCircularRect.moveLeft(indicatorRect.right() - indicatorCircularSize.width() - 2);
    }

    //文字
    QString str = q_ptr->text();
    QFontMetrics metrics = option.fontMetrics;
    QSize textSize = metrics.size(Qt::TextHideMnemonic, str);
    QPoint textLeftTop(indicatorRect.right() + ItemSpacing, rect.top() + (rect.height() - textSize.height()) / 2);
    textRect = QRect(textLeftTop, textSize);
}

}  // namespace KS
