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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/ui/box/modify-password.h"
#include <QMessageBox>
#include "src/ui/ui_modify-password.h"

namespace KS
{
ModifyPassword::ModifyPassword(QWidget *parent) : QWidget(parent),
                                                  m_ui(new Ui::ModifyPassword())
{
    this->m_ui->setupUi(this);
    this->init();
}

QString ModifyPassword::getCurrentPassword()
{
    return this->m_ui->m_currentPassword->text();
}

QString ModifyPassword::getNewPassword()
{
    return this->m_ui->m_newPassword->text();
}

void ModifyPassword::setBoxName(const QString boxName)
{
    this->m_ui->m_boxName->setText(boxName);
}

void ModifyPassword::init()
{
    this->setWindowModality(Qt::ApplicationModal);

    connect(this->m_ui->pushButton_2, &QPushButton::clicked, this, [this] {
        this->hide();
        emit rejected();
    });

    connect(this->m_ui->pushButton, &QPushButton::clicked, this, &ModifyPassword::onOkClicked);
}

void ModifyPassword::onOkClicked()
{
    if (this->m_ui->m_newPassword->text() != this->m_ui->m_confirmPassword->text())
    {
        QWidget *widget = new QWidget();
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
        emit accepted();
        this->hide();
        this->m_ui->m_currentPassword->setText("");
        this->m_ui->m_newPassword->setText("");
        this->m_ui->m_confirmPassword->setText("");
    }
}
}  // namespace KS
