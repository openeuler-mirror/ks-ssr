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
#include "trusted-user-pin.h"
#include "ui_trusted-user-pin.h"

namespace KS
{
TrustedUserPin::TrustedUserPin(QWidget *parent) : TitlebarWindow(parent),
                                                  m_ui(new Ui::TrustedUserPin)
{
    m_ui->setupUi(getWindowContentWidget());

    m_type = KSCKSSTrustedStorageType::KSC_KSS_TRUSTED_STORAGE_TYPE_NONE;

    initUI();
}

TrustedUserPin::~TrustedUserPin()
{
    delete m_ui;
}

QString TrustedUserPin::getUserPin()
{
    return m_ui->m_userPin->text();
}

KSCKSSTrustedStorageType TrustedUserPin::getType()
{
    return m_type;
}

void TrustedUserPin::setType(uint type)
{
    m_type = KSCKSSTrustedStorageType(type);
}

void TrustedUserPin::closeEvent(QCloseEvent *event)
{
    emit closed();
    QWidget::closeEvent(event);
}

void TrustedUserPin::initUI()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setTitle(tr("Input pin code"));
    setResizeable(false);
    setTitleBarHeight(36);
    setFixedSize(319, 259);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);

    connect(m_ui->m_cancel, &QPushButton::clicked, this, &TrustedUserPin::close);

    connect(m_ui->m_ok, &QPushButton::clicked, this, [this]
            {
                close();
                emit accepted();
                m_ui->m_userPin->setText("");
            });
}
}  // namespace KS
