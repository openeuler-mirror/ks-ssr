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
#include "modify-security-context.h"
#include "ui_modify-security-context.h"
#include <QPushButton>
#include <QIcon>

namespace KS
{
namespace ToolBox
{
ModifySecurityContext::ModifySecurityContext(QWidget *parent)
    : TitlebarWindow(parent),
      m_ui(new Ui::ModifySecurityContext)
{
    m_ui->setupUi(getWindowContentWidget());

    initUI();
}

ModifySecurityContext::~ModifySecurityContext()
{
    delete m_ui;
}

QString ModifySecurityContext::getSecurityContext() const
{
    return m_ui->m_securityContext->text();
}

void ModifySecurityContext::setFilePath(const QString &path)
{
    m_filePath = path;
}

QString ModifySecurityContext::getFilePath() const
{
    return m_filePath;
}

void ModifySecurityContext::setSecurityContext(const QString &text)
{
    return m_ui->m_securityContext->setText(text);
}

void ModifySecurityContext::closeEvent(QCloseEvent *event)
{
    emit closed();
    QWidget::closeEvent(event);
}

void ModifySecurityContext::initUI()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setTitle(tr("modify security context"));
    setResizeable(false);
    setTitleBarHeight(36);
    setFixedSize(319, 259);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);

    connect(m_ui->m_cancel, &QPushButton::clicked, this, &ModifySecurityContext::close);

    connect(m_ui->m_ok, &QPushButton::clicked, this, [this]
            {
                close();
                emit accepted();
                m_ui->m_securityContext->setText("");
            });
}
}  // namespace ToolBox
}  // namespace KS
