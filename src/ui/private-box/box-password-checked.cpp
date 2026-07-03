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
 * Author:     chendingjian <chendingjian@kylinos.com.cn>
 */
#include "box-password-checked.h"
#include <QRegularExpressionValidator>
#include "common/password-event-filter.h"
#include "include/ssr-i.h"
#include "ui_box-password-checked.h"
namespace KS
{
namespace PrivateBox
{
BoxPasswordChecked::BoxPasswordChecked(QWidget *parent)
    : TitlebarWindow(parent),
      m_ui(new Ui::BoxPasswordChecked)
{
    m_ui->setupUi(getWindowContentWidget());
    init();
}

BoxPasswordChecked::~BoxPasswordChecked()
{
    delete m_ui;
}

QString BoxPasswordChecked::getBoxPasswordChecked()
{
    return m_ui->m_inputPasswd->text();
}

void BoxPasswordChecked::init()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    setFixedSize(319, 259);

    auto validator = new QRegularExpressionValidator(QRegularExpression("[^ ]*"), this);
    m_ui->m_inputPasswd->setValidator(validator);
    m_ui->m_inputPasswd->setEchoMode(QLineEdit::Password);
    m_ui->m_inputPasswd->setMaxLength(SSR_PASSWORD_MAX_LENGTH);
    m_ui->m_inputPasswd->setContextMenuPolicy(Qt::NoContextMenu);
    m_ui->m_inputPasswd->installEventFilter(new PasswordEventFilter(m_ui->m_inputPasswd));
    connect(m_ui->m_cancel, &QPushButton::clicked, this, [this]
            {
                close();
                emit rejected();
            });

    connect(m_ui->m_ok, &QPushButton::clicked, this, [this]
            {
                close();
                emit accepted();
                m_ui->m_inputPasswd->setText("");
            });
}
}  // namespace PrivateBox
}  // namespace KS
