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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "src/ui/dm/device-list-page.h"
#include <kiran-log/qt5-log-i.h>
#include <QPainter>
#include <QPushButton>
#include <QWidgetAction>
#include "include/ssr-i.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/dm/device-permission.h"
#include "src/ui/dm/table-filter-model.h"
#include "src/ui/ui_device-list-page.h"

namespace KS
{
namespace DM
{
DeviceListPage::DeviceListPage(QWidget *parent) : Page(parent),
                                                  m_ui(new Ui::DeviceListPage),
                                                  m_devicePermission(nullptr),
                                                  m_deviceManagerProxy(nullptr)
{
    m_ui->setupUi(this);
    m_ui->m_title->setText(tr("Device List"));

    //设置搜索框搜索图标
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setObjectName("searchButton");
    searchButton->setIcon(QIcon(":/images/search"));
    searchButton->setIconSize(QSize(16, 16));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);

    //获取设备列表数据插入表格
    update();

    m_deviceManagerProxy = new DeviceManagerProxy(SSR_DBUS_NAME,
                                                  SSR_DEVICE_MANAGER_DBUS_OBJECT_PATH,
                                                  QDBusConnection::systemBus(),
                                                  this);

    connect(m_deviceManagerProxy, &DeviceManagerProxy::DeviceChanged, this, &DeviceListPage::update);
    connect(m_ui->m_search, &QLineEdit::textChanged, this, [this](const QString &text) { m_ui->m_table->setSearchText(text); });
    connect(m_ui->m_table, &DeviceListTable::clicked, this, &DeviceListPage::popupEditDialog);
}

DeviceListPage::~DeviceListPage()
{
    delete m_ui;
}

void DeviceListPage::update()
{
    m_ui->m_table->update();

    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_ui->m_table->getRowCount());
    m_ui->m_records->setText(text);
}

QString DeviceListPage::getNavigationUID()
{
    return tr("Device management");
}

QString DeviceListPage::getSidebarUID()
{
    return tr("Device List");
}

QString DeviceListPage::getSidebarIcon()
{
    return ":/images/device-list";
}

QString DeviceListPage::getAccountRoleName()
{
    return SSR_ACCOUNT_NAME_SYSADM;
}

void DeviceListPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DeviceListPage::popupEditDialog(const QModelIndex &index)
{
    RETURN_IF_TRUE(index.column() != m_ui->m_table->getColCount() - 1)

    m_devicePermission = new DevicePermission(this);
    connect(m_devicePermission, &DevicePermission::permissionChanged, this, &DeviceListPage::updatePermission);
    connect(m_devicePermission, &DevicePermission::stateChanged, this, &DeviceListPage::updateState);
    connect(m_devicePermission, &DevicePermission::deviceChanged, this, &DeviceListPage::update);

    auto deviceName = m_ui->m_table->getName(index.row());
    auto deviceID = m_ui->m_table->getID(index.row());
    auto state = m_ui->m_table->getState(index.row());
    auto permissions = m_ui->m_table->getPermission(index.row());
    auto type = m_ui->m_table->getType(index.row());

    m_devicePermission->setTitle(deviceName);
    m_devicePermission->setDeviceID(deviceID);
    m_devicePermission->setDeviceStatus(state);
    m_devicePermission->setDevicePermission(type, permissions);

    int x = this->x() + this->width() / 4 + m_devicePermission->width() / 4;
    int y = this->y() + this->height() / 4 + m_devicePermission->height() / 4;
    m_devicePermission->move(x, y);
    m_devicePermission->show();
}

void DeviceListPage::updatePermission()
{
    auto id = m_devicePermission->getDeviceID();
    //获取用户选择的设备权限
    auto permissions = m_devicePermission->getDevicePermission();

    //数据传入后台
    QJsonDocument jsonDoc;
    QJsonObject jsonObj{
        {SSR_DEVICE_JK_READ, (permissions & PermissionType::PERMISSION_TYPE_READ) > 0},
        {SSR_DEVICE_JK_WRITE, (permissions & PermissionType::PERMISSION_TYPE_WRITE) > 0},
        {SSR_DEVICE_JK_EXECUTE, (permissions & PermissionType::PERMISSION_TYPE_EXEC) > 0}};
    jsonDoc.setObject(jsonObj);

    auto reply = m_deviceManagerProxy->ChangePermission(id, QString(jsonDoc.toJson(QJsonDocument::Compact)));
    reply.waitForFinished();
    if (reply.isError())
    {
        POPUP_MESSAGE_DIALOG(reply.error().message());
        return;
    }
}

void DeviceListPage::updateState()
{
    auto id = m_devicePermission->getDeviceID();
    //获取用户选择的状态
    auto state = m_devicePermission->getDeviceStatus();
    RETURN_IF_TRUE(state != DeviceState::DEVICE_STATE_ENABLE && state != DeviceState::DEVICE_STATE_DISABLE);

    QDBusPendingReply<> reply;

    //数据传入后台
    if (state == DeviceState::DEVICE_STATE_ENABLE)
    {
        reply = m_deviceManagerProxy->Enable(id);
    }
    else
    {
        reply = m_deviceManagerProxy->Disable(id);
    }
    reply.waitForFinished();
    if (reply.isError())
    {
        POPUP_MESSAGE_DIALOG(reply.error().message());
        // 更新状态已经失败了，不需要再继续往下执行updatePermission的操作
        disconnect(m_devicePermission, &DevicePermission::permissionChanged, this, &DeviceListPage::updatePermission);
        return;
    }
}
}  // namespace DM
}  // namespace KS
