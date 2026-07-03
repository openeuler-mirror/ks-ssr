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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "src/ui/device/sidebar-item.h"
#include "src/ui/ui_sidebar-item.h"

#include <QPainter>
#include <QStyleOption>

namespace KS
{
SidebarItem::SidebarItem(const QString &text,
                         const QString &icon,
                         QWidget *parent) : QWidget(parent),
                                            m_ui(new Ui::SidebarItem),
                                            m_isSelected(false)
{
    m_ui->setupUi(this);
    m_ui->m_text->setText(text);
    m_ui->m_icon->setPixmap(QPixmap(icon));
    m_ui->m_arrow->setPixmap(QPixmap(":/images/arrow-right"));
    m_ui->m_arrow->hide();
}

SidebarItem::~SidebarItem()
{
    delete m_ui;
}

void SidebarItem::setSelected(bool isSelected)
{
    m_isSelected = isSelected;
    if (m_isSelected)
    {
        m_ui->m_arrow->show();
    }
    else
    {
        m_ui->m_arrow->hide();
    }
}

bool SidebarItem::isSelected()
{
    return m_isSelected;
}

void SidebarItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

}  // namespace KS
