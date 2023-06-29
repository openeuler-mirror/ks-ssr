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

#include "device-permission.h"
#include <kiran-log/qt5-log-i.h>
#include <QLineEdit>
#include <QListView>
#include <QPainter>
#include <QStyledItemDelegate>
#include "src/ui/common/ksc-marcos-ui.h"
#include "src/ui/device/device-utils.h"
#include "ui_device-permission.h"

namespace KS
{
DevicePermission::DevicePermission(QWidget *parent) : TitlebarWindow(parent),
                                                      m_ui(new Ui::DevicePermission)
{
    m_ui->setupUi(getWindowContentWidget());
    setIcon(QIcon(":/images/logo"));
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose);

    //给QCombobox设置代理才能设置下拉列表项的高度
    auto delegate = new QStyledItemDelegate(this);
    m_ui->m_status->setItemDelegate(delegate);

    m_ui->m_status->addItem(tr("enable"), DeviceState::DEVICE_STATE_ENABLE);
    m_ui->m_status->addItem(tr("disable"), DeviceState::DEVICE_STATE_DISABLE);

    connect(m_ui->m_confirm, &QPushButton::clicked, this, &DevicePermission::confirm);
    connect(m_ui->m_cancel, &QPushButton::clicked, this, &DevicePermission::close);
    connect(m_ui->m_status, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DevicePermission::update);
}

DevicePermission::~DevicePermission()
{
    delete m_ui;
}

QString DevicePermission::getDeviceID()
{
    return m_id;
}

void DevicePermission::setDeviceID(const QString &id)
{
    m_id = id;
}

void DevicePermission::setDeviceStatus(const DeviceState &status)
{
    m_status = status;
    switch (m_status)
    {
    case DeviceState::DEVICE_STATE_UNAUTHORIED:
    {
        //由于qt5 .15.2及以上的版本设置QCombobox占位符，无法正常显示，见QTBUG - 90595，因此使用自定义QLineEdit来显示占位符
        auto line = new QLineEdit(m_ui->m_status);
        line->setObjectName("lineEdit");
        line->setPlaceholderText(tr("Please select device status"));
        line->setReadOnly(true);
        line->installEventFilter(this);
        m_ui->m_status->setLineEdit(line);
        m_ui->m_status->lineEdit()->clear();
        m_ui->m_status->setCurrentIndex(-1);
        break;
    }
    case DeviceState::DEVICE_STATE_ENABLE:
    case DeviceState::DEVICE_STATE_DISABLE:
    {
        int currIndex = m_ui->m_status->findData(m_status);
        m_ui->m_status->setCurrentIndex(currIndex);
        break;
    }
    default:
        break;
    }

    if (m_status != DeviceState::DEVICE_STATE_ENABLE)
    {
        m_ui->m_groupBox->setDisabled(true);
    }
}

void DevicePermission::setDevicePermission(const QString type, int permission)
{
    m_permissions = permission;

    //针对挂载的存储设备，默认有可读权限，并且用户无法取消勾选
    if (type == DeviceUtils::deviceTypeEnum2Str(DeviceType::DEVICE_TYPE_DISK))
    {
        m_ui->m_read->setChecked(true);
        m_ui->m_read->setDisabled(true);
    }
    else
    {
        m_ui->m_read->setChecked(m_permissions & PERMISSION_TYPE_READ);
        m_ui->m_read->setDisabled(false);
    }
    m_ui->m_write->setChecked(m_permissions & PERMISSION_TYPE_WRITE);
    m_ui->m_exec->setChecked(m_permissions & PERMISSION_TYPE_EXEC);
}

DeviceState DevicePermission::getDeviceStatus()
{
    return m_status;
}

int DevicePermission::getDevicePermission()
{
    return m_permissions;
}

void DevicePermission::confirm()
{
    int permissions = 0;
    if (m_ui->m_read->isChecked())
    {
        permissions |= PERMISSION_TYPE_READ;
    }
    if (m_ui->m_write->isChecked())
    {
        permissions |= PERMISSION_TYPE_WRITE;
    }
    if (m_ui->m_exec->isChecked())
    {
        permissions |= PERMISSION_TYPE_EXEC;
    }
    auto state = (DeviceState)m_ui->m_status->currentData().toInt();

    if (0 == permissions && state == DeviceState::DEVICE_STATE_ENABLE)
    {
        POPUP_MESSAGE_DIALOG(tr("Please select at least one permission."));
        return;
    }

    if (state != m_status)
    {
        m_status = state;
        emit stateChanged();
    }

    if (permissions != m_permissions)
    {
        //禁用状态下无法修改权限，只有在启用状态下才能修改权限
        if (state == DeviceState::DEVICE_STATE_ENABLE)
        {
            m_permissions = permissions;
            emit permissionChanged();
        }
    }

    if (state != m_status || permissions != m_permissions)
    {
        emit deviceChanged();
    }

    hide();
}

void DevicePermission::update(int index)
{
    //设备未授权状态下不能修改权限和状态
    m_ui->m_confirm->setDisabled(0 > index);

    auto state = (DeviceState)m_ui->m_status->currentData().toInt();
    m_ui->m_groupBox->setDisabled(state != DeviceState::DEVICE_STATE_ENABLE);

    //若选择禁用，还原权限
    if (state != DeviceState::DEVICE_STATE_ENABLE)
    {
        m_ui->m_read->setChecked(m_permissions & PERMISSION_TYPE_READ);
        m_ui->m_write->setChecked(m_permissions & PERMISSION_TYPE_WRITE);
        m_ui->m_exec->setChecked(m_permissions & PERMISSION_TYPE_EXEC);
    }
}

void DevicePermission::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

bool DevicePermission::eventFilter(QObject *watched, QEvent *event)
{
    RETURN_VAL_IF_FALSE(watched == m_ui->m_status->lineEdit(), false);
    if (event->type() == QEvent::MouseButtonRelease)
    {
        m_ui->m_status->showPopup();
        return true;
    }
    //由于QCombobox在鼠标双击后会选中输入框的文字 ，因此过滤鼠标双击事件，不进行任何处理，避免选中情况
    else if (event->type() == QEvent::MouseButtonDblClick)
    {
        return true;
    }
    return false;
}

}  // namespace KS

//  namespace KS
