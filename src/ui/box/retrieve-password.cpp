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
    this->init();
    this->initStyle();
}

RetrievePassword::~RetrievePassword()
{
    delete m_ui;
}

QString RetrievePassword::getNewPassword()
{
    return this->m_ui->m_password->text();
}

QString RetrievePassword::getPassphrase()
{
    return this->m_ui->m_passphrase->text();
}

void RetrievePassword::init()
{
    this->setWindowModality(Qt::ApplicationModal);
    this->setWindowTitle(tr("Retrieve password"));
    this->m_ui->m_password->setEchoMode(QLineEdit::Password);
    this->m_ui->m_confirmPassword->setEchoMode(QLineEdit::Password);
    this->m_ui->m_passphrase->setEchoMode(QLineEdit::Password);
    connect(this->m_ui->m_cancel, &QPushButton::clicked, this, [this]
            {
                this->close();
                emit this->rejected();
            });

    connect(this->m_ui->m_ok, &QPushButton::clicked, this, &RetrievePassword::onOkClicked);
}

void RetrievePassword::initStyle()
{
    this->m_ui->m_ok->setFixedSize(80, 36);
    this->m_ui->m_ok->setStyleSheet("QPushButton{"
                                    "color:#FFFFFF;"
                                    "font:NotoSansCJKsc-Regular;"
                                    "font-size:12px;"
                                    "border-radius:8px;"
                                    "background:#43A3F2;}"
                                    "QPushButton:hover{"
                                    "background:#79C3FF;"
                                    "border:4px;}");

    this->m_ui->m_cancel->setFixedSize(80, 36);
    this->m_ui->m_cancel->setStyleSheet("QPushButton{"
                                        "color:#FFFFFF;"
                                        "font:NotoSansCJKsc-Regular;"
                                        "font-size:12px;"
                                        "border-radius:8px;"
                                        "background:#393939;}"
                                        "QPushButton:hover{"
                                        "background:#464646;"
                                        "border:4px;}");

    this->m_ui->m_password->setFixedHeight(36);
    this->m_ui->m_passphrase->setFixedHeight(36);
    this->m_ui->m_confirmPassword->setFixedHeight(36);
}

void RetrievePassword::onOkClicked()
{
    // 禁止输入空字符
    if (this->m_ui->m_password->text().isEmpty() || this->m_ui->m_passphrase->text().isEmpty() || this->m_ui->m_confirmPassword->text().isEmpty())
    {
        emit this->inputEmpty();
        return;
    }
    // 两次密码不一致
    if (this->m_ui->m_password->text() != this->m_ui->m_confirmPassword->text())
    {
        emit this->passwdInconsistent();
        return;
    }

    emit accepted();
    this->close();
    this->m_ui->m_passphrase->setText("");
    this->m_ui->m_password->setText("");
    this->m_ui->m_confirmPassword->setText("");
}
}  // namespace KS
