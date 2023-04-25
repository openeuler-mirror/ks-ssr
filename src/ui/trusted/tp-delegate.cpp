#include "tp-delegate.h"
#include <qt5-log-i.h>
#include <QPainterPath>
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

namespace KS
{
// 表格每行线条绘制的的圆角半径
#define TABLE_LINE_RADIUS 4

TPDelegate::TPDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

TPDelegate::~TPDelegate()
{
    KLOG_DEBUG() << "The TPDelegate is deleted.";
}

void TPDelegate::paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const
{
    painter->save();

    QPainterPath path;
    painter->setRenderHint(QPainter::RenderHint::Antialiasing);
    if (index.column() == 0)
    {
        auto rect = option.rect.adjusted(0, 2, TABLE_LINE_RADIUS, -2);
        path.addRoundedRect(rect, TABLE_LINE_RADIUS, TABLE_LINE_RADIUS);
    }
    else if (index.column() == index.model()->columnCount(index.parent()) - 1)
    {
        auto rect = option.rect.adjusted(-TABLE_LINE_RADIUS, 2, 0, -2);
        path.addRoundedRect(rect, TABLE_LINE_RADIUS, TABLE_LINE_RADIUS);
    }
    else
    {
        auto rect = option.rect.adjusted(0, 2, 0, -2);
        path.addRect(rect);
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(57, 57, 57)));
    painter->drawPath(path);

    painter->restore();

    if (index.column() == 0)
    {
        auto checkboxOption = option;
        this->initStyleOption(&checkboxOption, index);

        QStyleOptionButton checkboxStyle;
        QPixmap pixmap;
        auto value = index.model()->data(index, Qt::EditRole).toBool();
        pixmap.load(value ? ":/images/checkbox-checked-normal" : ":/images/checkbox-unchecked-normal");
        checkboxStyle.state = value ? QStyle::State_On : QStyle::State_Off;
        checkboxStyle.state |= QStyle::State_Enabled;
        checkboxStyle.iconSize = QSize(20, 20);
        checkboxStyle.rect = option.rect;
        checkboxStyle.rect.setX(option.rect.x() + 22);

        const QWidget *widget = option.widget;
        auto *style = widget ? widget->style() : QApplication::style();
        //        style->drawControl(QStyle::CE_CheckBox, &checkboxStyle, painter);
        style->drawItemPixmap(painter, option.rect, Qt::AlignCenter, pixmap);
    }
    else
    {
        this->QStyledItemDelegate::paint(painter, option, index);
    }
}

bool TPDelegate::editorEvent(QEvent *event,
                             QAbstractItemModel *model,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index)
{
    auto docorationRect = option.rect;
    auto mouseEvent = static_cast<QMouseEvent *>(event);

    if (event->type() == QEvent::MouseButtonPress &&
        docorationRect.contains(mouseEvent->pos()) &&
        index.column() == 0)
    {
        auto value = model->data(index, Qt::EditRole).toBool();
        model->setData(index, !value, Qt::EditRole);
    }

    return this->QStyledItemDelegate::editorEvent(event, model, option, index);
}
}  // namespace KS
