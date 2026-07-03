/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     chendingjian <chendingjian@kylinsec.com.cn>
 */

#include "tp-user-pin.h"
#include "ui_tp-user-pin.h"

namespace KS
{
namespace TP
{
TPUserPin::TPUserPin(QWidget *parent)
    : TitlebarWindow(parent),
      m_ui(new Ui::TPUserPin)
{
    m_ui->setupUi(getWindowContentWidget());

    m_type = SSRKSSTrustedStorageType::SSR_KSS_TRUSTED_STORAGE_TYPE_NONE;

    initUI();
}

TPUserPin::~TPUserPin()
{
    delete m_ui;
}

QString TPUserPin::getUserPin()
{
    return m_ui->m_userPin->text();
}

SSRKSSTrustedStorageType TPUserPin::getType()
{
    return m_type;
}

void TPUserPin::setType(uint type)
{
    m_type = SSRKSSTrustedStorageType(type);
}

void TPUserPin::closeEvent(QCloseEvent *event)
{
    emit closed();
    QWidget::closeEvent(event);
}

void TPUserPin::initUI()
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

    connect(m_ui->m_cancel, &QPushButton::clicked, this, &TPUserPin::close);

    connect(m_ui->m_ok, &QPushButton::clicked, this, [this]
            {
                close();
                emit accepted();
                m_ui->m_userPin->setText("");
            });
}
}  // namespace TP
}  // namespace KS
