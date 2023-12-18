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

#include "src/ui/common/password-modification.h"
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include "include/ssr-i.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/ui_password-modification.h"

namespace KS
{
PasswordModification::PasswordModification(QWidget *parent) : TitlebarWindow(parent),
                                                              m_ui(new Ui::PasswordModification())
{
    m_ui->setupUi(getWindowContentWidget());
    init();
}

QString PasswordModification::getCurrentPassword()
{
    return m_ui->m_currentPassword->text();
}

QString PasswordModification::getNewPassword()
{
    return m_ui->m_newPassword->text();
}

void PasswordModification::setTitleNameTail(const QString &tail)
{
    setTitle(QString(tr("Modify Password - %1").arg(tail)));
}

void PasswordModification::init()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    setMinimumSize(419, 419);

    auto validator = new QRegularExpressionValidator(QRegularExpression("[^ ]*"), this);
    m_ui->m_currentPassword->setValidator(validator);
    m_ui->m_newPassword->setValidator(validator);
    m_ui->m_confirmPassword->setValidator(validator);

    // 限制字符
    m_ui->m_currentPassword->setMaxLength(SSR_USER_NAME_MAX_LENGTH);
    m_ui->m_newPassword->setMaxLength(SSR_PASSWORD_MAX_LENGTH);
    m_ui->m_confirmPassword->setMaxLength(SSR_PASSWORD_MAX_LENGTH);

    m_ui->m_currentPassword->setEchoMode(QLineEdit::Password);
    m_ui->m_newPassword->setEchoMode(QLineEdit::Password);
    m_ui->m_confirmPassword->setEchoMode(QLineEdit::Password);
    connect(m_ui->m_cancel, &QPushButton::clicked, this, [this] {
        close();
        emit rejected();
    });

    connect(m_ui->m_ok, &QPushButton::clicked, this, &PasswordModification::acceptedPasswordModification);
}

void PasswordModification::acceptedPasswordModification()
{
    // 禁止输入空字符
    if (m_ui->m_newPassword->text().isEmpty() ||
        m_ui->m_confirmPassword->text().isEmpty() ||
        m_ui->m_currentPassword->text().isEmpty())
    {
        POPUP_MESSAGE_DIALOG(tr("The input cannot be empty, please improve the information."));
        return;
    }
    // 两次密码不一致
    if (m_ui->m_newPassword->text() != m_ui->m_confirmPassword->text())
    {
        POPUP_MESSAGE_DIALOG(QString(tr("Please confirm whether the password is consistent.")));
        return;
    }

    emit accepted();
    close();
    m_ui->m_currentPassword->setText("");
    m_ui->m_newPassword->setText("");
    m_ui->m_confirmPassword->setText("");
}
}  // namespace KS
