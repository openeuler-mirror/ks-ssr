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

#include "src/ui/peripheral-management/pm-page.h"
#include "src/ui/ui_pm-page.h"
#include "src/ui/peripheral-management/sidebar-item.h"
#include "src/ui/peripheral-management/pm-connect-page.h"
#include "src/ui/peripheral-management/pm-device-page.h"

#include <QListWidgetItem>
#include <QPainter>

namespace KS
{

PMPage::PMPage(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::PMPage)
{
    this->m_ui->setupUi(this);
    initSidebar();
    initSubPage();

    connect(m_ui->m_sidebar,&QListWidget::currentRowChanged, m_ui->m_stacked,&QStackedWidget::setCurrentIndex);
}

PMPage::~PMPage()
{
    delete this->m_ui;
}

void PMPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void PMPage::initSidebar()
{
    createSideBarItem(tr("Device List"),"");
    createSideBarItem(tr("Connect Record"),"");
    m_ui->m_sidebar->setCurrentRow(0);
}

void PMPage::initSubPage()
{
    PMDevicePage *deviceList = new PMDevicePage(this);
    PMConnectPage *connectRecord = new PMConnectPage(this);

    m_ui->m_stacked->addWidget(deviceList);
    m_ui->m_stacked->addWidget(connectRecord);
}

void PMPage::createSideBarItem(const QString text, const QString icon)
{
    QListWidgetItem *newItem = nullptr;
    SidebarItem *customItem = nullptr;

    newItem = new QListWidgetItem(m_ui->m_sidebar);
    customItem = new SidebarItem(text,icon,m_ui->m_sidebar);

    m_ui->m_sidebar->addItem(newItem);
    m_ui->m_sidebar->setItemWidget(newItem,customItem);

    newItem->setTextAlignment(Qt::AlignVCenter);
    newItem->setSizeHint(QSize(166,50));

    m_ui->m_sidebar->setGridSize(QSize(166,66));
    newItem->setData(Qt::UserRole,text);
}

}  // namespace KS
