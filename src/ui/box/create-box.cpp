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

#include "src/ui/box/create-box.h"
#include "src/ui/ui_create-box.h"

namespace KS
{
CreateBox::CreateBox(QWidget *parent) : QWidget(parent),
                                        m_ui(new Ui::CreateBox())
{
    this->m_ui->setupUi(this);
    this->setWindowModality(Qt::ApplicationModal);
    this->m_ui->m_password->setEchoMode(QLineEdit::Password);
    this->m_ui->m_confirmPassword->setEchoMode(QLineEdit::Password);
    connect(this->m_ui->m_ok, &QPushButton::clicked, this, &CreateBox::onOkClicked);

    connect(this->m_ui->m_cancel, &QPushButton::clicked, this, [this](bool)
            {
                Q_EMIT this->rejected();
                this->close();
            });
}

QString CreateBox::getName()
{
    return this->m_ui->m_name->text();
}

QString CreateBox::getPassword()
{
    return this->m_ui->m_password->text();
}

void CreateBox::onOkClicked()
{
    if (this->m_ui->m_password->text() != this->m_ui->m_confirmPassword->text())
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
        this->m_ui->m_name->setText("");
        this->m_ui->m_password->setText("");
        this->m_ui->m_confirmPassword->setText("");
    }
};

}  // namespace KS
