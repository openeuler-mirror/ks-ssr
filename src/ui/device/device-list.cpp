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
#include <kiran-log/qt5-log-i.h>
#include <QEvent>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include "include/ksc-i.h"
#include "include/ksc-marcos.h"
#include "src/ui/device/device-enum-utils.h"
#include "src/ui/device/device-permission.h"
#include "src/ui/device/table-filter-model.h"
#include "src/ui/ui_device-list.h"
namespace KS
{
DeviceList::DeviceList(QWidget *parent) : QWidget(parent),
                                          m_ui(new Ui::DeviceList),
                                          m_devicePermission(nullptr),
                                          m_deviceManagerProxy(nullptr)
{
    m_ui->setupUi(this);
    m_ui->m_title->setText(tr("Device List"));
    m_ui->m_search->addAction(QIcon(":/images/search"), QLineEdit::ActionPosition::LeadingPosition);

    //获取设备列表数据插入表格
    update();

    m_deviceManagerProxy = new DeviceManagerProxy(KSC_DBUS_NAME,
                                                  KSC_DEVICE_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);

    connect(m_ui->m_search, &QLineEdit::textChanged, this, &DeviceList::searchTextChanged);
    connect(m_ui->m_table, &DeviceListTable::clicked, this, &DeviceList::popupEditDialog);
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

void DeviceList::update()
{
    m_ui->m_table->update();

    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_ui->m_table->getRowCount());
    m_ui->m_records->setText(text);
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

void DeviceList::popupEditDialog(const QModelIndex &index)
{
    if (index.column() == m_ui->m_table->getColCount() - 1)
    {
        auto deviceName = m_ui->m_table->getName(index.row());
        auto deviceID = m_ui->m_table->getID(index.row());
        auto state = m_ui->m_table->getState(index.row());
        auto permissions = m_ui->m_table->getPermission(index.row());

        if (!m_devicePermission)
        {
            m_devicePermission = new DevicePermission(deviceName, deviceID, this);
            connect(m_devicePermission, &DevicePermission::permissionChanged, this, &DeviceList::updatePermission);
            connect(m_devicePermission, &DevicePermission::destroyed,
                    [this]
                    {
                        m_devicePermission->deleteLater();
                        m_devicePermission = nullptr;
                    });
        }
        m_devicePermission->setDeviceStatus(state);
        m_devicePermission->setDevicePermission(permissions);

        int x = this->x() + this->width() / 4 + m_devicePermission->width() / 4;
        int y = this->y() + this->height() / 4 + m_devicePermission->height() / 4;
        this->m_devicePermission->move(x, y);
        this->m_devicePermission->show();
    }
}

void DeviceList::updatePermission()
{
    //获取用户选择的设备权限和状态
    auto status = m_devicePermission->getDeviceStatus();
    auto permissions = m_devicePermission->getDevicePermission();
    auto id = m_devicePermission->getDeviceID();

    //TODO:将数据传入后台
    QJsonDocument jsonDoc;
    QJsonObject jsonObj{
        {KSC_DEVICE_KEY_ID, id},
        {KSC_DEVICE_KEY_READ, permissions & PermissionType::PERMISSION_TYPE_READ},
        {KSC_DEVICE_KEY_WRITE, permissions & PermissionType::PERMISSION_TYPE_WRITE},
        {KSC_DEVICE_KEY_EXECUTE, permissions & PermissionType::PERMISSION_TYPE_EXEC},
        {KSC_DEVICE_KEY_STATE, status}};
    jsonDoc.setObject(jsonObj);

    KLOG_DEBUG() << "change permission json:" << QString(jsonDoc.toJson(QJsonDocument::Compact));
    m_deviceManagerProxy->ChangePermission(QString(jsonDoc.toJson(QJsonDocument::Compact)));

    //更新表格
    update();
}
}  //namespace KS
