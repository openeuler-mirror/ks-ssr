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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/ui/box/box-password-modification.h"
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include "include/ssr-i.h"
#include "src/ui/ui_box-password-modification.h"

namespace KS
{
BoxPasswordModification::BoxPasswordModification(QWidget *parent) : TitlebarWindow(parent),
                                                                    m_ui(new Ui::BoxPasswordModification())
{
    m_ui->setupUi(getWindowContentWidget());
    init();
}

QString BoxPasswordModification::getCurrentPassword()
{
    return m_ui->m_currentPassword->text();
}

QString BoxPasswordModification::getNewPassword()
{
    return m_ui->m_newPassword->text();
}

void BoxPasswordModification::setBoxName(const QString &boxName)
{
    m_ui->m_boxName->setText(boxName);
}

void BoxPasswordModification::init()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);

    auto validator = new QRegularExpressionValidator(QRegularExpression("[^ ]*"), this);
    m_ui->m_currentPassword->setValidator(validator);
    m_ui->m_newPassword->setValidator(validator);
    m_ui->m_confirmPassword->setValidator(validator);

    // 限制字符
    m_ui->m_currentPassword->setMaxLength(SSR_BOX_PASSWORD_MAX_LENGTH);
    m_ui->m_newPassword->setMaxLength(SSR_BOX_PASSWORD_MAX_LENGTH);
    m_ui->m_confirmPassword->setMaxLength(SSR_BOX_PASSWORD_MAX_LENGTH);

    m_ui->m_currentPassword->setEchoMode(QLineEdit::Password);
    m_ui->m_newPassword->setEchoMode(QLineEdit::Password);
    m_ui->m_confirmPassword->setEchoMode(QLineEdit::Password);
    connect(m_ui->m_cancel, &QPushButton::clicked, this, [this]
            {
                close();
                emit rejected();
            });

    connect(m_ui->m_ok, &QPushButton::clicked, this, &BoxPasswordModification::onOkClicked);
}

void BoxPasswordModification::onOkClicked()
{
    // 禁止输入空字符
    if (m_ui->m_newPassword->text().isEmpty() ||
        m_ui->m_confirmPassword->text().isEmpty() ||
        m_ui->m_currentPassword->text().isEmpty())
    {
        emit inputEmpty();
        return;
    }
    // 两次密码不一致
    if (m_ui->m_newPassword->text() != m_ui->m_confirmPassword->text())
    {
        emit passwdInconsistent();
        return;
    }

    emit accepted();
    close();
    m_ui->m_currentPassword->setText("");
    m_ui->m_newPassword->setText("");
    m_ui->m_confirmPassword->setText("");
}
}  // namespace KS
