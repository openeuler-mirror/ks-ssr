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

#include "src/ui/device/device-list-page.h"
#include "src/ui/device/device-permission.h"
#include "src/ui/ui_device-list-page.h"

#include <kiran-log/qt5-log-i.h>
#include <QPainter>

namespace KS
{
DeviceListPage::DeviceListPage(QWidget *parent) : QWidget(parent),
                                                  m_ui(new Ui::DeviceListPage),
                                                  m_devicePermission(nullptr)
{
    m_ui->setupUi(this);
    m_ui->m_title->setText(tr("Device List"));

    m_ui->m_search->addAction(QIcon(":/images/search"), QLineEdit::ActionPosition::LeadingPosition);
    connect(m_ui->m_search, &QLineEdit::textChanged, this, &DeviceListPage::searchTextChanged);
    connect(m_ui->m_edit, &QPushButton::clicked, this, &DeviceListPage::editClicked);
}

DeviceListPage::~DeviceListPage()
{
    delete m_ui;
    if (m_devicePermission)
    {
        delete m_devicePermission;
        m_devicePermission = nullptr;
    }
}

void DeviceListPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DeviceListPage::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    auto filterProxy = this->m_ui->m_table->getFilterProxy();
    filterProxy->setFilterFixedString(text);
}

void DeviceListPage::editClicked(bool checked)
{
    if (!m_devicePermission)
    {
        m_devicePermission = new DevicePermission("test");
        connect(m_devicePermission, &DevicePermission::okCliecked, this, &DeviceListPage::updateDevice);
        connect(m_devicePermission, &DevicePermission::destroyed,
                [this]
                {
                    m_devicePermission->deleteLater();
                    m_devicePermission = nullptr;
                });
    }

    QList<PMPermissionsType> permissions;
    permissions.append(PM_PERMISSIONS_TYPE_READ);
    m_devicePermission->setDeviceStatus(PM_DEVICE_STATUS_UNACTIVE);
    m_devicePermission->setDevicePermission(permissions);

    int x = this->x() + this->width() / 4 + m_devicePermission->width() / 4;
    int y = this->y() + this->height() / 4 + m_devicePermission->height() / 4;
    this->m_devicePermission->move(x, y);
    this->m_devicePermission->show();
}

void DeviceListPage::updateDevice()
{
    auto status = m_devicePermission->getDeviceStatus();
    auto permissions = m_devicePermission->getDevicePermission();
}
}  //namespace KS
