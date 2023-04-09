/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * kiran-session-manager is licensed under Mulan PSL v2.
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

    connect(this->m_ui->m_ok, &QPushButton::clicked, this, [this](bool) {
        // TODO: 检查数据合法性
        Q_EMIT this->accepted();
    });

    connect(this->m_ui->m_cancel, &QPushButton::clicked, this, [this](bool) {
        Q_EMIT this->rejected();
    });
}

QString CreateBox::getName()
{
    return this->m_ui->m_name->text();
}

QString CreateBox::getPassword()
{
    return this->m_ui->m_password->text();
};

}  // namespace KS
