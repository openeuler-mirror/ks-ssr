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

#include "table-header-proxy.h"

#include <QCheckBox>
#include <QMouseEvent>
#include <QPainter>
#include "src/ui/common/table/header-button-delegate.h"
#include "include/ssr-marcos.h"

namespace KS
{
TableHeaderProxy::TableHeaderProxy(QWidget *parent) : QHeaderView(Qt::Horizontal, parent),
                                                      m_stateChanged(false),
                                                      m_closeCheckBox(false),
                                                      m_checkState(Qt::Unchecked),
                                                      m_headerButtons({})
{
    m_rect = new QRect(15, 2, 20, 20);
    // 做下拉筛选功能时可能会用到这个属性，暂设置为false
    setSectionsClickable(false);
    setObjectName("tableHeaderProxy");
}

void TableHeaderProxy::hideCheckBox(bool isHide)
{
    m_closeCheckBox = isHide;
}

void TableHeaderProxy::setHeaderButtons(QMap<int, HeaderButtonDelegate *> headerButtons)
{
    m_headerButtons = headerButtons;
}

void TableHeaderProxy::paintSection(QPainter *painter,
                                    const QRect &rect,
                                    int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (logicalIndex == 0 && !m_closeCheckBox)
    {
        QPixmap pixmap;
        switch (m_checkState)
        {
        case Qt::CheckState::Checked:
            pixmap.load(":/images/checkbox-checked-normal");
            break;
        case Qt::CheckState::Unchecked:
            pixmap.load(":/images/checkbox-unchecked-normal");
            break;
        case Qt::CheckState::PartiallyChecked:
            pixmap.load(":/images/checkbox-indeterminate-normal");
            break;
        default:
            break;
        }

        style()->drawItemPixmap(painter, *m_rect, Qt::AlignCenter, pixmap);
    }
//    RETURN_IF_TRUE(m_arrowCols.isEmpty());
    for (auto key : m_headerButtons.keys())
    {
        if (logicalIndex == key)
        {
            m_headerButtons.value(key)->setGeometry(rect);
        }
    }
}

void TableHeaderProxy::mousePressEvent(QMouseEvent *e)
{
    auto column = logicalIndexAt(e->pos());
    if (column == 0 && !m_closeCheckBox)
    {
        if ((e->pos().x() > m_rect->x()) && (e->pos().y() > m_rect->y()) && (e->pos().x() < m_rect->x() + 20) && (e->pos().y() < m_rect->y() + 20))
        {
            m_checkState = m_checkState == Qt::Unchecked ? Qt::Checked : Qt::Unchecked;
            emit toggled(m_checkState);
            viewport()->update();
        }
    }
    QHeaderView::mousePressEvent(e);
}

void TableHeaderProxy::mouseMoveEvent(QMouseEvent *e)
{
    e->ignore();
}

void TableHeaderProxy::setCheckState(Qt::CheckState checkState)
{
    m_stateChanged = true;
    m_checkState = checkState;
    update();
}
}  // namespace KS
