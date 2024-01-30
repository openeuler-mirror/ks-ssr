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
#include "add-user-dialog.h"
#include <QRegularExpressionValidator>
#include <QIcon>
#include "include/ssr-i.h"
#include "ui_add-user-dialog.h"
namespace KS
{
namespace ToolBox
{
AddUserDialog::AddUserDialog(QWidget *parent)
    : TitlebarWindow(parent),
      m_ui(new Ui::AddUserDialog)
{
    m_ui->setupUi(getWindowContentWidget());
    init();
}

AddUserDialog::~AddUserDialog()
{
    delete m_ui;
}

QStringList AddUserDialog::getUserList() const
{
    return m_ui->m_input->text().split(Qt::Key_Semicolon);
}

void AddUserDialog::init()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    setFixedSize(319, 259);

    auto validator = new QRegularExpressionValidator(QRegularExpression("[a-zA-Z0-9_.][a-zA-Z0-9_.-]*[$]?([;][a-zA-Z0-9_.][a-zA-Z0-9_.-]*[$]?)*"), this);
    m_ui->m_input->setValidator(validator);
    connect(m_ui->m_cancel, &QPushButton::clicked, this, [this]
            {
                close();
                emit rejected();
            });

    connect(m_ui->m_ok, &QPushButton::clicked, this, [this]
            {
                close();
                emit accepted();
                m_ui->m_input->setText("");
            });
}
}  // namespace ToolBox
}  // namespace KS
