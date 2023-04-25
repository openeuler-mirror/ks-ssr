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
#include "input-password.h"
#include "ui_input-password.h"
namespace KS
{
InputPassword::InputPassword(QWidget *parent) : QWidget(parent),
                                                m_ui(new Ui::InputPassword)
{
    m_ui->setupUi(this);
    this->init();
}

InputPassword::~InputPassword()
{
    delete m_ui;
}

QString InputPassword::getInputPassword()
{
    return this->m_ui->m_inputPasswd->text();
}

void InputPassword::init()
{
    this->setWindowModality(Qt::ApplicationModal);
    m_ui->m_inputPasswd->setEchoMode(QLineEdit::Password);
    connect(this->m_ui->m_cancel, &QPushButton::clicked, this, [this]
            {
                this->close();
                emit this->rejected();
            });

    connect(this->m_ui->m_ok, &QPushButton::clicked, this, [this]
            {
                m_ui->m_inputPasswd->setText("");
                this->close();
                emit this->accepted();
            });
}

}  // namespace KS
