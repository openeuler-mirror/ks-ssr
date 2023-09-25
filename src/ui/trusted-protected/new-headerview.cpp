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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */

#include "new-headerview.h"

#include <QMouseEvent>
#include <QPainter>

namespace KS
{
NewHeaderView::NewHeaderView(QWidget *parent) : QHeaderView(Qt::Horizontal, parent),
                                                m_stateChanged(false),
                                                m_checkState(Qt::Unchecked)
{
    m_rect = new QRect(15, 2, 20, 20);
    // 做下拉筛选功能时可能会用到这个属性，暂设置为false
    this->setSectionsClickable(false);
    this->setMouseTracking(true);
}

void NewHeaderView::paintSection(QPainter *painter,
                                 const QRect &rect,
                                 int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (logicalIndex == 0)
    {
        QPixmap pix;
        switch (m_checkState)
        {
        case Qt::CheckState::Checked:
            pix.load(":/images/checkbox-checked-normal");
            break;
        case Qt::CheckState::Unchecked:
            pix.load(":/images/checkbox-unchecked-normal");
            break;
        case Qt::CheckState::PartiallyChecked:
            pix.load(":/images/checkbox-indeterminate-normal");
            break;
        default:
            break;
        }

        style()->drawItemPixmap(painter, *m_rect, Qt::AlignCenter, pix);
    }
}

void NewHeaderView::mousePressEvent(QMouseEvent *e)
{
    auto column = logicalIndexAt(e->pos());
    if (column == 0)
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

void NewHeaderView::setCheckState(Qt::CheckState checkState)
{
    m_stateChanged = true;
    m_checkState = checkState;
    update();
}

}  // namespace KS