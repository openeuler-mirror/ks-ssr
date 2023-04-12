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

namespace KS {


EditPermissions::EditPermissions(QWidget *parent) :
    TitlebarWindow(parent),
    m_ui(new Ui::EditPermissions)
{
    m_ui->setupUi(getWindowContentWidget());
    setTitle("Edit permissions");

    m_ui->m_cb_status->addItem(tr("enable"),PM_DEVICE_STATUS_ENABLE);
    m_ui->m_cb_status->addItem(tr("disable"),PM_DEVICE_STATUS_DISABLE);
    m_ui->m_cb_status->addItem(tr("unactive"),PM_DEVICE_STATUS_UNACTIVE);
    m_ui->m_cb_status->setCurrentIndex(0);

    m_ui->m_btn_read->setChecked(true);
    connect(m_ui->m_confirm,&QPushButton::clicked,this,&EditPermissions::onConfirm);
}

EditPermissions::~EditPermissions()
{
    delete m_ui;
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

        QLabel *label = new QLabel(QString(tr("Please confirm whether the password is consistent.")));
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
        hide();
        m_ui->m_cb_status->setCurrentIndex(0);
        m_ui->m_btn_read->setChecked(true);
        m_ui->m_btn_write->setChecked(false);
        m_ui->m_btn_exec->setChecked(false);
    }
}

}

//  namespace KS
