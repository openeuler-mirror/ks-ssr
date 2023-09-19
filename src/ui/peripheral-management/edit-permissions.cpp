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

#include "edit-permissions.h"
#include "ui_edit-permissions.h"
#include <QPainter>
#include <QLineEdit>

#include <kiran-log/qt5-log-i.h>

namespace KS {


EditPermissions::EditPermissions(const QString name, const PMDeviceStatus status, const QList<PMPermissionsType> permission, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::EditPermissions),
    m_name(name),
    m_status(status),
    m_permission(permission)
{
    m_ui->setupUi(this);
    setWindowTitle(name);
    setWindowFlags(Qt::Window);
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose);

    initComboBox();
    initBtns();
    connect(m_ui->m_confirm,&QPushButton::clicked,this,&EditPermissions::onConfirm);
    connect(m_ui->m_cancel,&QPushButton::clicked,this,&EditPermissions::close);
}

EditPermissions::~EditPermissions()
{
    delete m_ui;
}

void EditPermissions::initComboBox()
{
    switch (m_status) {
    case PM_DEVICE_STATUS_UNACTIVE:
    {
        QLineEdit * line = new QLineEdit(m_ui->m_cb_status);
        line->setPlaceholderText(tr("Please select device status"));
        line->setReadOnly(true);
        m_ui->m_cb_status->setLineEdit(line);
        m_ui->m_cb_status->addItem(tr("enable"),PM_DEVICE_STATUS_ENABLE);
        m_ui->m_cb_status->addItem(tr("disable"),PM_DEVICE_STATUS_DISABLE);
        m_ui->m_cb_status->lineEdit()->clear();
        break;
    }
    case PM_DEVICE_STATUS_ENABLE:
    case PM_DEVICE_STATUS_DISABLE:
    {
        m_ui->m_cb_status->addItem(tr("enable"),PM_DEVICE_STATUS_ENABLE);
        m_ui->m_cb_status->addItem(tr("disable"),PM_DEVICE_STATUS_DISABLE);

        int currIndex = m_ui->m_cb_status->findData(m_status);
        m_ui->m_cb_status->setCurrentIndex(currIndex);
    }
    default:
        break;
    }

    connect(m_ui->m_cb_status,QOverload<int>::of(&QComboBox::activated),this,&EditPermissions::onStatusChanged);
}

void EditPermissions::initBtns()
{
    if(m_status != PM_DEVICE_STATUS_ENABLE)
    {
        m_ui->m_groupBox->setDisabled(true);
    }
    //TODO:后续看禁用状态下是否设置初始值
    else
    {
        foreach (PMPermissionsType pemission, m_permission)
        {
            if(pemission == PM_PERMISSIONS_TYPE_READ)
                m_ui->m_btn_read->setChecked(true);
            else if(pemission == PM_PERMISSIONS_TYPE_WRITE)
                m_ui->m_btn_write->setChecked(true);
            else if(pemission == PM_PERMISSIONS_TYPE_EXEC)
                m_ui->m_btn_exec->setChecked(true);
        }
    }
}


void EditPermissions::onConfirm()
{
    QList<PMPermissionsType> permissions;
    if(m_ui->m_btn_read->isChecked())
        permissions.append(PM_PERMISSIONS_TYPE_READ);
    if(m_ui->m_btn_write->isChecked())
        permissions.append(PM_PERMISSIONS_TYPE_WRITE);
    if(m_ui->m_btn_exec->isChecked())
        permissions.append(PM_PERMISSIONS_TYPE_EXEC);

    if(permissions.isEmpty())
    {
        QWidget *widget = new QWidget();
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->setWindowModality(Qt::ApplicationModal);
        widget->setWindowTitle(tr("notice"));
        widget->setWindowIcon(QIcon(":/images/logo"));
        QVBoxLayout *vlay = new QVBoxLayout(widget);

        QLabel *label = new QLabel(QString(tr("Please select at least one permission.")));
        QPushButton *ok = new QPushButton(tr("ok"));
        connect(ok, &QPushButton::clicked, widget, &QWidget::close);

        vlay->addWidget(label);
        vlay->addWidget(ok);

        int x = this->x() + this->width() / 4 + widget->width() / 4;
        int y = this->y() + this->height() / 4 + widget->height() / 4;
        widget->move(x, y);
        widget->show();
    }
    else
    {
        auto status = m_ui->m_cb_status->currentData().toInt();

        emit permissionChanged(status,permissions);
        close();
        m_ui->m_cb_status->setCurrentIndex(0);
        m_ui->m_btn_read->setChecked(true);
        m_ui->m_btn_write->setChecked(false);
        m_ui->m_btn_exec->setChecked(false);
    }
}

void EditPermissions::onStatusChanged(int index)
{
    KLOG_DEBUG() << "status changed to :" << index;
    if(m_ui->m_cb_status->currentData().toInt() != PM_DEVICE_STATUS_ENABLE)
    {
        m_ui->m_groupBox->setDisabled(true);
    }
    else
    {
        m_ui->m_groupBox->setDisabled(false);
    }

}

void EditPermissions::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

}

//  namespace KS
