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

#include "src/ui/device/device-page.h"
#include "src/ui/device/device-list.h"
#include "src/ui/device/device-log.h"
#include "src/ui/device/sidebar-item.h"
#include "src/ui/ui_device-page.h"

#include <QListWidgetItem>
#include <QPainter>

namespace KS
{
DevicePage::DevicePage(QWidget *parent) : QWidget(parent),
                                          m_ui(new Ui::DevicePage)
{
    this->m_ui->setupUi(this);
    initSidebar();
    initSubPage();
}

DevicePage::~DevicePage()
{
    delete this->m_ui;
}

void DevicePage::update()
{
    DeviceList *listPage = qobject_cast<DeviceList *>(m_ui->m_stacked->widget(DevicePageType::DEVICE_PAGE_TYPE_LIST));
    listPage->update();

    DeviceLog *logPage = qobject_cast<DeviceLog *>(m_ui->m_stacked->widget(DevicePageType::DEVICE_PAGE_TYPE_LOG));
    logPage->update();
}

void DevicePage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DevicePage::initSidebar()
{
    createSideBarItem(tr("Device List"), ":/images/device-list");
    createSideBarItem(tr("Device Log"), ":/images/device-log");

    connect(m_ui->m_sidebar, &QListWidget::currentRowChanged, m_ui->m_stacked, &QStackedWidget::setCurrentIndex);
    connect(m_ui->m_sidebar, &QListWidget::currentRowChanged, this, &DevicePage::setSideBarItemStatus);
    m_ui->m_sidebar->setCurrentRow(0);
}

void DevicePage::initSubPage()
{
    auto deviceList = new DeviceList(this);
    auto deviceLog = new DeviceLog(this);

    m_ui->m_stacked->addWidget(deviceList);
    m_ui->m_stacked->addWidget(deviceLog);
}

void DevicePage::createSideBarItem(const QString &text, const QString &icon)
{
    auto newItem = new QListWidgetItem(m_ui->m_sidebar);
    auto customItem = new SidebarItem(text, icon, m_ui->m_sidebar);

    m_ui->m_sidebar->addItem(newItem);
    m_ui->m_sidebar->setItemWidget(newItem, customItem);

    newItem->setTextAlignment(Qt::AlignVCenter);
    newItem->setSizeHint(QSize(166, 50));

    m_ui->m_sidebar->setGridSize(QSize(166, 66));
    newItem->setData(Qt::UserRole, text);
}

void DevicePage::setSideBarItemStatus(int currentRow)
{
    //更新侧边栏item状态
    for (auto i = 0; i < m_ui->m_sidebar->count(); i++)
    {
        auto customItem = qobject_cast<SidebarItem *>(m_ui->m_sidebar->itemWidget(m_ui->m_sidebar->item(i)));
        if (i == currentRow)
            customItem->setSelected(true);
        else
            customItem->setSelected(false);
    }
}

}  // namespace KS
