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
#include "kernel-protected-delegate.h"
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace KS
{
namespace TP
{
// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

KernelProtectedDelegate::KernelProtectedDelegate(QObject *parent)
    : Delegate(parent)
{
}

void KernelProtectedDelegate::paint(QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QPainterPath path;
    if (index.column() == index.model()->columnCount(index.parent()) - 1)
    {
        auto rect = option.rect.adjusted(-TABLE_LINE_RADIUS, 2, 0, -2);
        path.addRoundedRect(rect, TABLE_LINE_RADIUS, TABLE_LINE_RADIUS);

        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(QColor(57, 57, 57)));
        painter->drawPath(path);
    }

    if (index.column() == PROHIBIT_UNLOADING_COL)
    {
        auto switchOption = option;
        initStyleOption(&switchOption, index);

        QStyleOptionButton switchStyle;
        QPixmap pixmap;
        auto value = index.model()->data(index, Qt::EditRole).toBool();
        pixmap.load(value ? ":/images/switch-open" : ":/images/switch-close");

        switchStyle.state = value ? QStyle::State_On : QStyle::State_Off;
        switchStyle.state |= QStyle::State_Enabled;
        switchStyle.iconSize = QSize(52, 24);
        switchStyle.rect = option.rect;
        switchStyle.rect.setX(option.rect.x() - 22);

        auto widget = option.widget;
        auto style = widget ? widget->style() : QApplication::style();
        style->drawItemPixmap(painter, switchStyle.rect, Qt::AlignCenter, pixmap);
    }
    else
    {
        Delegate::paint(painter, option, index);
    }
}

bool KernelProtectedDelegate::editorEvent(QEvent *event,
                                          QAbstractItemModel *model,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index)
{
    auto docorationRect = option.rect;
    auto mouseEvent = static_cast<QMouseEvent *>(event);

    if (event->type() == QEvent::MouseButtonPress &&
        docorationRect.contains(mouseEvent->pos()) &&
        index.column() == PROHIBIT_UNLOADING_COL)
    {
        auto value = model->data(index, Qt::EditRole).toBool();
        model->setData(index, !value, Qt::EditRole);
    }

    return Delegate::editorEvent(event, model, option, index);
}

}  // namespace TP
}  // namespace KS