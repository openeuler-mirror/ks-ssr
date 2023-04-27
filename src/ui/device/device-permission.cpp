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
#include "include/ksc-marcos.h"
#include "ui_device-permission.h"

namespace KS
{
DevicePermission::DevicePermission(const QString &name, QWidget *parent) : TitlebarWindow(parent),
                                                                           m_ui(new Ui::DevicePermission),
                                                                           m_name(name)
{
    m_ui->setupUi(getWindowContentWidget());
    setTitle(name);
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose);

    //给QCombobox设置代理才能设置下拉列表项的高度
    auto delegate = new QStyledItemDelegate(this);
    m_ui->m_status->setItemDelegate(delegate);

    m_ui->m_status->addItem(tr("enable"), DEVICE_STATUS_ENABLE);
    m_ui->m_status->addItem(tr("disable"), DEVICE_STATUS_DISABLE);

    connect(m_ui->m_confirm, &QPushButton::clicked, this, &DevicePermission::confirm);
    connect(m_ui->m_cancel, &QPushButton::clicked, this, &DevicePermission::close);
    connect(m_ui->m_status, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DevicePermission::updateGroupBox);
}

DevicePermission::~DevicePermission()
{
    delete m_ui;
}

void DevicePermission::setDeviceStatus(const DeviceStatus &status)
{
    m_status = status;
    switch (m_status)
    {
    case DEVICE_STATUS_UNACTIVE:
    {
        //由于qt5 .15.2及以上的版本设置QCombobox占位符，无法正常显示，见QTBUG - 90595，因此使用自定义QLineEdit来显示占位符
        auto line = new QLineEdit(m_ui->m_status);
        line->setPlaceholderText(tr("Please select device status"));
        line->setReadOnly(true);
        line->installEventFilter(this);
        m_ui->m_status->setLineEdit(line);
        m_ui->m_status->lineEdit()->clear();
        m_ui->m_status->setCurrentIndex(-1);
        break;
    }
    case DEVICE_STATUS_ENABLE:
    case DEVICE_STATUS_DISABLE:
    {
        int currIndex = m_ui->m_status->findData(m_status);
        m_ui->m_status->setCurrentIndex(currIndex);
        break;
    }
    default:
        break;
    }

    if (m_status != DEVICE_STATUS_ENABLE)
    {
        m_ui->m_groupBox->setDisabled(true);
    }
}

void DevicePermission::setDevicePermission(int permission)
{
    m_permissions = permission;

    m_ui->m_read->setChecked(m_permissions & DEVICE_PERMISSION_TYPE_READ);
    m_ui->m_write->setChecked(m_permissions & DEVICE_PERMISSION_TYPE_WRITE);
    m_ui->m_exec->setChecked(m_permissions & DEVICE_PERMISSION_TYPE_EXEC);
}

DeviceStatus DevicePermission::getDeviceStatus()
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
        permissions |= DEVICE_PERMISSION_TYPE_READ;
    }
    if (m_ui->m_write->isChecked())
    {
        permissions |= DEVICE_PERMISSION_TYPE_WRITE;
    }
    if (m_ui->m_exec->isChecked())
    {
        permissions |= DEVICE_PERMISSION_TYPE_EXEC;
    }

    if (0 == permissions)
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
        m_status = DeviceStatus(status);
        m_permissions = permissions;

        emit permissionChanged();
        hide();
    }
}

void DevicePermission::updateGroupBox(int index)
{
    if (m_ui->m_status->currentData().toInt() != DEVICE_STATUS_ENABLE)
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

bool DevicePermission::eventFilter(QObject *watched, QEvent *event)
{
    RETURN_VAL_IF_FALSE(watched == m_ui->m_status->lineEdit() && event->type() == QEvent::MouseButtonRelease, false);

    m_ui->m_status->showPopup();
    return true;
}

}  // namespace KS

//  namespace KS
