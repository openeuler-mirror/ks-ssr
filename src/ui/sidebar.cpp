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

#include "src/ui/sidebar.h"
#include "src/ui/ui_sidebar.h"
#include "ssr-marcos.h"

#include <QPainter>
#include <QStyleOption>

namespace KS
{
SidebarItem::SidebarItem(const ItemInfo &itemInfo,
                         QWidget *parent) : QWidget(parent),
                                            m_ui(new Ui::SidebarItem),
                                            m_isSelected(false)
{
    m_ui->setupUi(this);
    m_ui->m_text->setText(itemInfo.name);
    m_ui->m_icon->setPixmap(QIcon(itemInfo.icon).pixmap(16, 16));
    m_ui->m_arrow->setPixmap(QIcon(":/images/right-arrow").pixmap(8, 10));
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

QString SidebarItem::getItemUID()
{
    return m_ui->m_text->text();
}

void SidebarItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

SideBar::SideBar(QWidget *parent) : QListWidget(parent)
{
    setFixedWidth(200);
    connect(this, &QListWidget::currentRowChanged, this, &SideBar::setSideBarItemStatus);
}

void SideBar::addSideBarItem(SidebarItem *item)
{
    auto newItem = new QListWidgetItem(this);

    addItem(newItem);
    setItemWidget(newItem, item);

    newItem->setTextAlignment(Qt::AlignVCenter);
    newItem->setSizeHint(QSize(166, 50));

    setGridSize(QSize(166, 66));
    newItem->setData(Qt::UserRole, item->getItemUID());
}

QString SideBar::getSelectedUID()
{
    auto customItem = qobject_cast<SidebarItem *>(this->itemWidget(this->item(this->currentRow())));
    return customItem->getItemUID();
}

void SideBar::setSideBarItemStatus(int currentRow)
{
    //更新侧边栏item状态
    for (auto i = 0; i < this->count(); i++)
    {
        auto customItem = qobject_cast<SidebarItem *>(this->itemWidget(this->item(i)));
        RETURN_IF_FALSE(customItem)
        if (i == currentRow)
            customItem->setSelected(true);
        else
            customItem->setSelected(false);
    }
    emit itemChanged();
}

}  // namespace KS
