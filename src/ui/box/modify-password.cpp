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

void ModifyPassword::setBoxName(const QString &boxName)
{
    this->m_ui->m_boxName->setText(boxName);
}

void ModifyPassword::init()
{
    this->setWindowModality(Qt::ApplicationModal);
    this->m_ui->m_currentPassword->setEchoMode(QLineEdit::Password);
    this->m_ui->m_newPassword->setEchoMode(QLineEdit::Password);
    this->m_ui->m_confirmPassword->setEchoMode(QLineEdit::Password);
    connect(this->m_ui->m_cancel, &QPushButton::clicked, this, [this]
            {
                this->close();
                emit this->rejected();
            });

    connect(this->m_ui->m_ok, &QPushButton::clicked, this, &ModifyPassword::onOkClicked);
}

void ModifyPassword::onOkClicked()
{
    // 禁止输入空字符
    if (this->m_ui->m_newPassword->text().isEmpty() ||
        this->m_ui->m_confirmPassword->text().isEmpty() ||
        this->m_ui->m_currentPassword->text().isEmpty())
    {
        emit this->inputEmpty();
        return;
    }
    // 两次密码不一致
    if (this->m_ui->m_newPassword->text() != this->m_ui->m_confirmPassword->text())
    {
        emit this->passwdInconsistent();
        return;
    }

    emit accepted();
    this->close();
    this->m_ui->m_currentPassword->setText("");
    this->m_ui->m_newPassword->setText("");
    this->m_ui->m_confirmPassword->setText("");
}
}  // namespace KS
