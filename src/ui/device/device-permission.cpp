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
#include <QLineEdit>
#include <QPainter>
#include "ui_device-permission.h"

#include <kiran-log/qt5-log-i.h>

namespace KS
{
DevicePermission::DevicePermission(const QString &name, QWidget *parent) : QWidget(parent),
                                                                           m_ui(new Ui::DevicePermission),
                                                                           m_name(name)
{
    m_ui->setupUi(this);
    setWindowTitle(name);
    setWindowFlags(Qt::Window);
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose);

    m_ui->m_status->addItem(tr("enable"), PM_DEVICE_STATUS_ENABLE);
    m_ui->m_status->addItem(tr("disable"), PM_DEVICE_STATUS_DISABLE);

    connect(m_ui->m_confirm, &QPushButton::clicked, this, &DevicePermission::onConfirm);
    connect(m_ui->m_cancel, &QPushButton::clicked, this, &DevicePermission::close);
    connect(m_ui->m_status, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DevicePermission::onStatusChanged);
}

DevicePermission::~DevicePermission()
{
    delete m_ui;
}

void DevicePermission::setDeviceStatus(PMDeviceStatus status)
{
    m_status = status;
    switch (m_status)
    {
    case PM_DEVICE_STATUS_UNACTIVE:
    {
        //由于qt5.15.2及以上的版本设置QCombobox占位符，无法正常显示，见QTBUG-90595，因此使用自定义QLineEdit来显示占位符
        auto *line = new QLineEdit(m_ui->m_status);
        line->setPlaceholderText(tr("Please select device status"));
        line->setReadOnly(true);
        m_ui->m_status->setLineEdit(line);
        m_ui->m_status->lineEdit()->clear();
        m_ui->m_status->setCurrentIndex(-1);
        break;
    }
    case PM_DEVICE_STATUS_ENABLE:
    case PM_DEVICE_STATUS_DISABLE:
    {
        int currIndex = m_ui->m_status->findData(m_status);
        m_ui->m_status->setCurrentIndex(currIndex);
        break;
    }
    default:
        break;
    }
}

void DevicePermission::setDevicePermission(QList<PMPermissionsType> permission)
{
    m_permissions = permission;
    if (m_status != PM_DEVICE_STATUS_ENABLE)
    {
        m_ui->m_groupBox->setDisabled(true);
    }
    //TODO:后续看未授权状态下是否设置初始值
    foreach (PMPermissionsType pemission, m_permissions)
    {
        if (pemission == PM_PERMISSIONS_TYPE_READ)
        {
            m_ui->m_read->setChecked(true);
        }
        else if (pemission == PM_PERMISSIONS_TYPE_WRITE)
        {
            m_ui->m_write->setChecked(true);
        }
        else if (pemission == PM_PERMISSIONS_TYPE_EXEC)
        {
            m_ui->m_exec->setChecked(true);
        }
    }
}

PMDeviceStatus DevicePermission::getDeviceStatus()
{
    return m_status;
}

QList<PMPermissionsType> DevicePermission::getDevicePermission()
{
    return m_permissions;
}

void DevicePermission::onConfirm()
{
    QList<PMPermissionsType> permissions;
    if (m_ui->m_read->isChecked())
    {
        permissions.append(PM_PERMISSIONS_TYPE_READ);
    }
    if (m_ui->m_write->isChecked())
    {
        permissions.append(PM_PERMISSIONS_TYPE_WRITE);
    }
    if (m_ui->m_exec->isChecked())
    {
        permissions.append(PM_PERMISSIONS_TYPE_EXEC);
    }

    if (permissions.isEmpty())
    {
        auto *msgDialog = new QWidget();
        msgDialog->setAttribute(Qt::WA_DeleteOnClose);
        msgDialog->setWindowModality(Qt::ApplicationModal);
        msgDialog->setWindowTitle(tr("notice"));
        msgDialog->setWindowIcon(QIcon(":/images/logo"));
        auto *vlay = new QVBoxLayout(msgDialog);

        auto *label = new QLabel(QString(tr("Please select at least one permission.")));
        auto *ok = new QPushButton(tr("ok"), msgDialog);
        connect(ok, &QPushButton::clicked, msgDialog, &QWidget::close);

        vlay->addWidget(label);
        vlay->addWidget(ok);

        int x = this->x() + this->width() / 4 + msgDialog->width() / 4;
        int y = this->y() + this->height() / 4 + msgDialog->height() / 4;
        msgDialog->move(x, y);
        msgDialog->show();
    }
    else
    {
        auto status = m_ui->m_status->currentData().toInt();
        m_status = int2enum(status);
        m_permissions = permissions;

        emit okCliecked();
        hide();
    }
}

void DevicePermission::onStatusChanged(int index)
{
    KLOG_DEBUG() << "status changed to :" << index;
    if (m_ui->m_status->currentData().toInt() != PM_DEVICE_STATUS_ENABLE)
    {
        m_ui->m_groupBox->setDisabled(true);
    }
    else
    {
        m_ui->m_groupBox->setDisabled(false);
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

PMDeviceStatus DevicePermission::int2enum(int status)
{
    switch (status)
    {
    case PM_DEVICE_STATUS_UNACTIVE:
        return PM_DEVICE_STATUS_UNACTIVE;
    case PM_DEVICE_STATUS_DISABLE:
        return PM_DEVICE_STATUS_DISABLE;
    case PM_DEVICE_STATUS_ENABLE:
        return PM_DEVICE_STATUS_ENABLE;
    default:
        return PM_DEVICE_STATUS_LAST;
    }
}

}  // namespace KS

//  namespace KS
