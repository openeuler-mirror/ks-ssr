#include "tp-kernel-delegate.h"
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace KS
{
// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

TPKernelDelegate::TPKernelDelegate(QObject *parent) : TPDelegate(parent)
{

}

void TPKernelDelegate::paint(QPainter *painter,
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
        TPDelegate::paint(painter, option, index);
    }
}

bool TPKernelDelegate::editorEvent(QEvent *event,
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

    return TPDelegate::editorEvent(event, model, option, index);
}

}  // namespace KS
