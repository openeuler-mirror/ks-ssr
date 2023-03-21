/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * kiran-session-manager is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/ui/box/flow-layout.h"
#include <QWidget>
#include "sc-marcos.h"

namespace KS
{
FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing) : QLayout(parent),
                                                                                  m_hSpace(hSpacing),
                                                                                  m_vSpace(vSpacing)

{
    this->setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = this->takeAt(0)))
    {
        delete item;
    }
}

void FlowLayout::addItem(QLayoutItem *item)
{
    this->m_itemList.append(item);
}

int FlowLayout::horizontalSpacing() const
{
    RETURN_VAL_IF_TRUE(this->m_hSpace >= 0, this->m_hSpace);

    return this->smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const
{
    RETURN_VAL_IF_TRUE(this->m_vSpace >= 0, this->m_vSpace);

    return this->smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int FlowLayout::count() const
{
    return this->m_itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return this->m_itemList.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < this->m_itemList.size())
    {
        return this->m_itemList.takeAt(index);
    }
    return nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {};
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    return this->doLayout(QRect(0, 0, width, 0));
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect);
}

QSize FlowLayout::sizeHint() const
{
    return this->minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (const QLayoutItem *item : qAsConst(this->m_itemList))
    {
        size = size.expandedTo(item->minimumSize());
    }

    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

int FlowLayout::doLayout(const QRect &rect) const
{
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    int boxTotalHeight = 0;

    //    KLOG_DEBUG() << "FlowLayout rect: " << rect;

    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);

    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (QLayoutItem *item : qAsConst(this->m_itemList))
    {
        const auto itemWidget = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1)
        {
            spaceX = itemWidget->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        }
        int spaceY = verticalSpacing();
        if (spaceY == -1)
        {
            spaceY = itemWidget->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
        }

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
        {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        // KLOG_DEBUG() << "item geometry: " << QPoint(x, y) << " , size: " << item->sizeHint();

        item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();

    RETURN_VAL_IF_TRUE(parent == nullptr, -1);

    if (parent->isWidgetType())
    {
        QWidget *parentWidget = static_cast<QWidget *>(parent);
        return parentWidget->style()->pixelMetric(pm, nullptr, parentWidget);
    }
    else
    {
        return static_cast<QLayout *>(parent)->spacing();
    }
}

}  // namespace KS
