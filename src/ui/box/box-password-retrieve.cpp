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
#include <QRegularExpressionValidator>
#include "ui_box-password-retrieve.h"

namespace KS
{
RetrieveBoxPassword::RetrieveBoxPassword(QWidget *parent) : TitlebarWindow(parent),
                                                            m_ui(new Ui::RetrieveBoxPassword)
{
    m_ui->setupUi(getWindowContentWidget());
    init();
}

RetrieveBoxPassword::~RetrieveBoxPassword()
{
    delete m_ui;
}

QString RetrieveBoxPassword::getPassphrase()
{
    return m_ui->m_passphrase->text();
}

void RetrieveBoxPassword::init()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);

    auto validator = new QRegularExpressionValidator(QRegularExpression("[^ ]*"), this);
    m_ui->m_passphrase->setValidator(validator);
    m_ui->m_passphrase->setEchoMode(QLineEdit::Password);
    m_ui->m_passphrase->setMaxLength(16);

    connect(m_ui->m_cancel, &QPushButton::clicked, this, [this]
            {
                close();
                emit rejected();
            });

    connect(m_ui->m_ok, &QPushButton::clicked, this, &RetrieveBoxPassword::onOkClicked);
}

void RetrieveBoxPassword::onOkClicked()
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
