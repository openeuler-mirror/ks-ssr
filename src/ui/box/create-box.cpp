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

#include "src/ui/box/create-box.h"
#include <qt5-log-i.h>
#include <QRegularExpressionValidator>
#include "include/ssr-i.h"
#include "src/ui/ui_create-box.h"

namespace KS
{
CreateBox::CreateBox(QWidget *parent) : TitlebarWindow(parent),
                                        m_ui(new Ui::CreateBox())
{
    m_ui->setupUi(getWindowContentWidget());
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);

    auto validator = new QRegularExpressionValidator(QRegularExpression("[^ ]*"), this);
    auto validatorName = new QRegularExpressionValidator(QRegularExpression("^[\u4E00-\u9FA5A-Za-z0-9_]+$"), this);
    m_ui->m_name->setValidator(validatorName);
    m_ui->m_password->setValidator(validator);
    m_ui->m_confirmPassword->setValidator(validator);

    // 限制输入长度
    m_ui->m_name->setMaxLength(SSR_BOX_NAME_MAX_LENGTH);
    m_ui->m_password->setMaxLength(SSR_BOX_PASSWORD_MAX_LENGTH);
    m_ui->m_confirmPassword->setMaxLength(SSR_BOX_PASSWORD_MAX_LENGTH);

    m_ui->m_password->setEchoMode(QLineEdit::Password);
    m_ui->m_confirmPassword->setEchoMode(QLineEdit::Password);
    connect(m_ui->m_ok, &QPushButton::clicked, this, &CreateBox::onOkClicked);

    connect(m_ui->m_cancel, &QPushButton::clicked, this, [this](bool)
            {
                Q_EMIT rejected();
                close();
            });
}

QString CreateBox::getName()
{
    return m_ui->m_name->text();
}

QString CreateBox::getPassword()
{
    return m_ui->m_password->text();
}

void CreateBox::onOkClicked()
{
    // 禁止出现空密码、空保险箱名
    if (m_ui->m_password->text().isEmpty() || m_ui->m_confirmPassword->text().isEmpty() || m_ui->m_name->text().isEmpty())
    {
        emit inputEmpty();
        KLOG_WARNING() << "The input cannot be empty, please improve the information.";
        return;
    }

    // 两次输入的密码不一致
    if (m_ui->m_password->text() != m_ui->m_confirmPassword->text())
    {
        emit passwdInconsistent();
        return;
    }

    emit accepted();
    // 创建成功后清空输入框
    hide();
    m_ui->m_name->setText("");
    m_ui->m_password->setText("");
    m_ui->m_confirmPassword->setText("");
};

}  // namespace KS
