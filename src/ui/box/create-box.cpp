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
#include <qt5-log-i.h>
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
    // 禁止出现空密码、空保险箱名
    if (this->m_ui->m_password->text().isEmpty() || this->m_ui->m_confirmPassword->text().isEmpty() || this->m_ui->m_name->text().isEmpty())
    {
        emit this->inputEmpty();
        KLOG_WARNING() << "The input cannot be empty, please improve the information.";
        return;
    }

    // 两次输入的密码不一致
    if (this->m_ui->m_password->text() != this->m_ui->m_confirmPassword->text())
    {
        emit this->passwdInconsistent();
        return;
    }

    emit this->accepted();
    // 创建成功后清空输入框
    this->hide();
    this->m_ui->m_name->setText("");
    this->m_ui->m_password->setText("");
    this->m_ui->m_confirmPassword->setText("");
};

}  // namespace KS
