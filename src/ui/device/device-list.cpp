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
#include <QPainter>
#include <QPushButton>
#include <QWidgetAction>
#include "include/ksc-i.h"
#include "src/ui/common/ksc-marcos-ui.h"
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

    //设置搜索框搜索图标
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setIcon(QIcon(":/images/search"));
    searchButton->setIconSize(QSize(16, 16));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);

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
    auto filterProxy = m_ui->m_table->getFilterProxy();
    filterProxy->setFilterFixedString(text);
}

void DeviceList::popupEditDialog(const QModelIndex &index)
{
    if (index.column() == m_ui->m_table->getColCount() - 1)
    {
        if (!m_devicePermission)
        {
            m_devicePermission = new DevicePermission(this);
            connect(m_devicePermission, &DevicePermission::permissionChanged, this, &DeviceList::updatePermission);
            connect(m_devicePermission, &DevicePermission::stateChanged, this, &DeviceList::updateState);
            connect(m_devicePermission, &DevicePermission::destroyed,
                    [this]
                    {
                        m_devicePermission->deleteLater();
                        m_devicePermission = nullptr;
                    });
        }

        auto deviceName = m_ui->m_table->getName(index.row());
        auto deviceID = m_ui->m_table->getID(index.row());
        auto state = m_ui->m_table->getState(index.row());
        auto permissions = m_ui->m_table->getPermission(index.row());

        m_devicePermission->setTitle(deviceName);
        m_devicePermission->setDeviceID(deviceID);
        m_devicePermission->setDeviceStatus(state);
        m_devicePermission->setDevicePermission(permissions);

        int x = this->x() + this->width() / 4 + m_devicePermission->width() / 4;
        int y = this->y() + this->height() / 4 + m_devicePermission->height() / 4;
        m_devicePermission->move(x, y);
        m_devicePermission->show();
    }
}

void DeviceList::updatePermission()
{
    auto id = m_devicePermission->getDeviceID();
    //获取用户选择的设备权限
    auto permissions = m_devicePermission->getDevicePermission();

    //数据传入后台
    QJsonDocument jsonDoc;
    QJsonObject jsonObj{
        {KSC_DEVICE_JK_READ, (permissions & PermissionType::PERMISSION_TYPE_READ) > 0},
        {KSC_DEVICE_JK_WRITE, (permissions & PermissionType::PERMISSION_TYPE_WRITE) > 0},
        {KSC_DEVICE_JK_EXECUTE, (permissions & PermissionType::PERMISSION_TYPE_EXEC) > 0}};
    jsonDoc.setObject(jsonObj);

    if (!m_deviceManagerProxy->ChangePermission(id, QString(jsonDoc.toJson(QJsonDocument::Compact))))
    {
        POPUP_MESSAGE_DIALOG(tr("Failed to change permission of device!"));
        return;
    }
}

void DeviceList::updateState()
{
    auto id = m_devicePermission->getDeviceID();
    //获取用户选择的状态
    auto state = m_devicePermission->getDeviceStatus();
    RETURN_IF_TRUE(state != DEVICE_STATE_ENABLE && state != DEVICE_STATE_DISABLE);

    QString errMsg;
    //数据传入后台
    if (state == DeviceState::DEVICE_STATE_ENABLE)
    {
        if (!m_deviceManagerProxy->Enable(id))
        {
            errMsg = tr("Failed to enable device!");
        }
    }
    else
    {
        if (!m_deviceManagerProxy->Disable(id))
        {
            errMsg = tr("Failed to disable device!");
        }
    }

    if (!errMsg.isEmpty())
    {
        POPUP_MESSAGE_DIALOG(errMsg);
        return;
    }
}
}  //namespace KS
