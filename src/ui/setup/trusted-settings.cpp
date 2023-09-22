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
#include "trusted-settings.h"
#include "ui_trusted-settings.h"
namespace KS
{
TrustedSettings::TrustedSettings(QWidget *parent) : QWidget(parent),
                                                    m_ui(new Ui::TrustedSettings)
{
    m_ui->setupUi(this);
    QIcon icon(":/images/switch-open");
    m_ui->m_switch->setCheckable(true);
    m_ui->m_switch->setIcon(QIcon(m_ui->m_switch->isChecked() ? ":/images/switch-open" : ":/images/switch-close"));
    m_ui->m_switch->setFixedSize(52, 24);
    m_ui->m_switch->setIconSize(QSize(52, 24));
    connect(m_ui->m_switch, &QPushButton::clicked, this, &TrustedSettings::switchChanged);
}

TrustedSettings::~TrustedSettings()
{
    delete m_ui;
}

void TrustedSettings::switchChanged(bool checked)
{
    m_ui->m_switch->setIcon(QIcon(checked ? ":/images/switch-open" : ":/images/switch-close"));
    m_ui->m_switch->setChecked(checked);

    emit trustedStatusChange(checked);
}
}  // namespace KS
