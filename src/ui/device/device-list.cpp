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

#include "src/ui/device/device-list.h"
#include "src/ui/device/device-permission.h"
#include "src/ui/ui_device-list.h"

#include <kiran-log/qt5-log-i.h>
#include <QPainter>

namespace KS
{
DeviceList::DeviceList(QWidget *parent) : QWidget(parent),
                                          m_ui(new Ui::DeviceList),
                                          m_devicePermission(nullptr)
{
    m_ui->setupUi(this);
    m_ui->m_title->setText(tr("Device List"));

    m_ui->m_search->addAction(QIcon(":/images/search"), QLineEdit::ActionPosition::LeadingPosition);
    connect(m_ui->m_search, &QLineEdit::textChanged, this, &DeviceList::searchTextChanged);
    connect(m_ui->m_edit, &QPushButton::clicked, this, &DeviceList::editClicked);
}

DeviceList::~DeviceList()
{
    delete m_ui;
    if (m_devicePermission)
    {
        delete m_devicePermission;
        m_devicePermission = nullptr;
    }
}

void DeviceList::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DeviceList::searchTextChanged(const QString &text)
{
    auto filterProxy = this->m_ui->m_table->getFilterProxy();
    filterProxy->setFilterFixedString(text);
}

void DeviceList::editClicked(bool checked)
{
    if (!m_devicePermission)
    {
        //TODO:由于现在还没有调用后台接口获取设备名，先设置test作为设备名，后续改为具体的设备名
        m_devicePermission = new DevicePermission("test");
        connect(m_devicePermission, &DevicePermission::clickConfirm, this, &DeviceList::updateDevice);
        connect(m_devicePermission, &DevicePermission::destroyed,
                [this]
                {
                    m_devicePermission->deleteLater();
                    m_devicePermission = nullptr;
                });
    }

    int permissions = DEVICE_PERMISSION_TYPE_READ | DEVICE_PERMISSION_TYPE_EXEC;
    m_devicePermission->setDeviceStatus(DEVICE_STATUS_UNACTIVE);
    m_devicePermission->setDevicePermission(permissions);

    int x = this->x() + this->width() / 4 + m_devicePermission->width() / 4;
    int y = this->y() + this->height() / 4 + m_devicePermission->height() / 4;
    this->m_devicePermission->move(x, y);
    this->m_devicePermission->show();
}

void DeviceList::updateDevice()
{
    //获取用户选择的设备权限和状态
    auto status = m_devicePermission->getDeviceStatus();
    auto permissions = m_devicePermission->getDevicePermission();

    //TODO:将数据传入后台

    //TODO:将数据更新至表格
}
}  //namespace KS
