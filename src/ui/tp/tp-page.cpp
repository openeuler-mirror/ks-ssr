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

#include "src/ui/tp/tp-page.h"
#include "config.h"
#include "ksc-marcos.h"
#include "src/ui/common/loading.h"
#include "src/ui/common/message-dialog.h"
#include "src/ui/device/sidebar-item.h"
#include "src/ui/tp/tp-execute.h"
#include "src/ui/tp/tp-kernel.h"
#include "src/ui/ui_tp-page.h"

#include <QListWidgetItem>
#include <QPainter>

namespace KS
{
TPPage::TPPage(QWidget *parent) : QWidget(parent),
                                  m_ui(new Ui::TPPage)
{
    m_ui->setupUi(this);
    initSidebar();
    initSubPage();

    connect(m_ui->m_sidebar, &QListWidget::currentRowChanged, m_ui->m_stacked, &QStackedWidget::setCurrentIndex);
    connect(m_ui->m_sidebar, &QListWidget::itemClicked, this, &TPPage::onItemClicked);
}

TPPage::~TPPage()
{
    delete m_ui;
}

void TPPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void TPPage::resizeEvent(QResizeEvent *event)
{
    if (m_loading)
    {
        m_loading->setAutoFillBackground(true);
        m_loading->setFixedSize(m_ui->m_stacked->size());
    }
}

void TPPage::initSidebar()
{
    m_sidebarItems.clear();
    createSideBarItem(tr("Execute protecked"), ":/images/execution-control");
    createSideBarItem(tr("Kernel protecked"), ":/images/kernel-module-protected");
    m_ui->m_sidebar->setCurrentRow(0);
    // 第一条选中
    m_sidebarItems.value(m_ui->m_sidebar->item(0)->data(Qt::UserRole).toString())->setSelected(true);
}

void TPPage::initSubPage()
{
    auto execute = new TPExecute(m_ui->m_stacked);
    connect(execute, &TPExecute::initFinished, this, [this]
            {
                m_loading->setVisible(false);
                m_ui->m_sidebar->setEnabled(true);
            });
    auto kernel = new TPKernel(m_ui->m_stacked);

    m_ui->m_stacked->addWidget(execute);
    m_ui->m_stacked->addWidget(kernel);

    checkTrustedLoadFinied(execute->getInitialized());
}

void TPPage::checkTrustedLoadFinied(bool initialized)
{
    m_loading = new Loading(m_ui->m_stacked);
    RETURN_IF_TRUE(initialized)

    if (!m_loading->isVisible())
    {
        m_loading->setVisible(true);
    }

    m_ui->m_sidebar->setEnabled(false);
}

void TPPage::createSideBarItem(const QString &text, const QString &icon)
{
    auto newItem = new QListWidgetItem(m_ui->m_sidebar);
    auto customItem = new SidebarItem(text, icon, m_ui->m_sidebar);

    m_ui->m_sidebar->addItem(newItem);
    m_ui->m_sidebar->setItemWidget(newItem, customItem);

    newItem->setTextAlignment(Qt::AlignVCenter);
    newItem->setSizeHint(QSize(166, 50));

    m_ui->m_sidebar->setGridSize(QSize(166, 66));
    newItem->setData(Qt::UserRole, text);
    m_sidebarItems.insert(text, customItem);
}

void TPPage::onItemClicked(QListWidgetItem *currItem)
{
    auto currData = currItem->data(Qt::UserRole).toString();

    for (auto it = m_sidebarItems.begin(); it != m_sidebarItems.end(); it++)
    {
        auto sidebar = it.value();
        sidebar->setSelected(false);
    }

    for (auto item : m_ui->m_sidebar->selectedItems())
    {
        if (currData == item->data(Qt::UserRole).toString())
        {
            m_sidebarItems.value(currData)->setSelected(true);
        }
    }
}

}  // namespace KS
