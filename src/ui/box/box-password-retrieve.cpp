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
#include "box-password-retrieve.h"
#include "ui_box-password-retrieve.h"

namespace KS
{
BoxPasswordRetrieve::BoxPasswordRetrieve(QWidget *parent) : TitlebarWindow(parent),
                                                            m_ui(new Ui::BoxPasswordRetrieve)
{
    m_ui->setupUi(getWindowContentWidget());
    init();
}

BoxPasswordRetrieve::~BoxPasswordRetrieve()
{
    delete m_ui;
}

QString BoxPasswordRetrieve::getPassphrase()
{
    return m_ui->m_passphrase->text();
}

void BoxPasswordRetrieve::init()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);

    m_ui->m_passphrase->setEchoMode(QLineEdit::Password);
    connect(m_ui->m_cancel, &QPushButton::clicked, this, [this]
            {
                close();
                emit rejected();
            });

    connect(m_ui->m_ok, &QPushButton::clicked, this, &BoxPasswordRetrieve::onOkClicked);
}

void BoxPasswordRetrieve::onOkClicked()
{
    // 禁止输入空字符
    if (m_ui->m_passphrase->text().isEmpty())
    {
        emit inputEmpty();
        return;
    }

    emit accepted();
    close();
    m_ui->m_passphrase->setText("");
}
}  // namespace KS