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

#include "src/ui/trusted/tp-page.h"
#include "config.h"
#include "ksc-marcos.h"
#include "src/ui/common/sub-window.h"
#include "src/ui/device/sidebar-item.h"
#include "src/ui/trusted/tp-execute.h"
#include "src/ui/trusted/tp-kernel.h"
#include "src/ui/ui_tp-page.h"

#include <QListWidgetItem>
#include <QPainter>
#include <QSettings>

namespace KS
{
// ini文件
#define KSC_INI_PATH KSC_INSTALL_DATADIR "/ksc.ini"
#define KSC_INI_KEY "ksc/initialized"

TPPage::TPPage(QWidget *parent) : QWidget(parent),
                                  m_ui(new Ui::TPPage)
{
    this->m_ui->setupUi(this);
    initSidebar();
    initSubPage();
    checkTrustedLoadFinied();

    connect(m_ui->m_sidebar, &QListWidget::currentRowChanged, m_ui->m_stacked, &QStackedWidget::setCurrentIndex);
    connect(m_ui->m_sidebar, &QListWidget::itemClicked, this, &TPPage::onItemClicked);
}

TPPage::~TPPage()
{
    delete this->m_ui;
}

void TPPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
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
    auto *execute = new TPExecute(m_ui->m_stacked);
    auto *kernel = new TPKernel(m_ui->m_stacked);

    m_ui->m_stacked->addWidget(execute);
    m_ui->m_stacked->addWidget(kernel);
}

void TPPage::checkTrustedLoadFinied()
{
    auto settings = new QSettings(KSC_INI_PATH, QSettings::IniFormat, this);
    RETURN_IF_TRUE(settings->value(KSC_INI_KEY).toInt() != 0)

    auto widget = new SubWindow(this);
    widget->setFixedSize(240, 180);
    widget->buildNotify(tr("Trusted data needs to be initialised,"
                           "please wait a few minutes to refresh."));

    int x = this->x() + this->width() / 4 + widget->width() / 4;
    int y = this->y() + this->height() / 4 + widget->height() / 4;
    widget->move(x, y);
    widget->show();
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
