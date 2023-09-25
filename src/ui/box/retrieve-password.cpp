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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */
#include "retrieve-password.h"
#include "ui_retrieve-password.h"

namespace KS
{
RetrievePassword::RetrievePassword(QWidget *parent) : QWidget(parent),
                                                      m_ui(new Ui::RetrievePassword)
{
    m_ui->setupUi(this);
    init();
}

RetrievePassword::~RetrievePassword()
{
    delete m_ui;
}

QString RetrievePassword::getNewPassword()
{
    return m_ui->m_password->text();
}

QString RetrievePassword::getPassphrase()
{
    return m_ui->m_passphrase->text();
}

void RetrievePassword::init()
{
    setWindowModality(Qt::ApplicationModal);
    m_ui->m_password->setEchoMode(QLineEdit::Password);
    m_ui->m_confirmPassword->setEchoMode(QLineEdit::Password);
    m_ui->m_passphrase->setEchoMode(QLineEdit::Password);
    connect(m_ui->m_cancel, &QPushButton::clicked, this, [this]
            {
                close();
                emit rejected();
            });

    connect(m_ui->m_ok, &QPushButton::clicked, this, &RetrievePassword::onOkClicked);
}

void RetrievePassword::onOkClicked()
{
    // 禁止输入空字符
    if (m_ui->m_password->text().isEmpty() ||
        m_ui->m_passphrase->text().isEmpty() ||
        m_ui->m_confirmPassword->text().isEmpty())
    {
        emit inputEmpty();
        return;
    }
    // 两次密码不一致
    if (m_ui->m_password->text() != m_ui->m_confirmPassword->text())
    {
        emit passwdInconsistent();
        return;
    }

    emit accepted();
    close();
    m_ui->m_passphrase->setText("");
    m_ui->m_password->setText("");
    m_ui->m_confirmPassword->setText("");
}
}  // namespace KS
