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
#include "delete-notify.h"
#include "ui_delete-notify.h"

namespace KS
{
DeleteNotify::DeleteNotify(QWidget *parent)
    : TitlebarWindow(parent),
      m_ui(new Ui::DeleteNotify)
{
    m_ui->setupUi(getWindowContentWidget());

    init();
}

DeleteNotify::~DeleteNotify()
{
    delete m_ui;
}

void DeleteNotify::setNotifyMessage(const QString &title, const QString &message)
{
    setTitle(title);
    m_ui->m_notify->setText(message);
}

void DeleteNotify::init()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    setFixedSize(299, 219);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    m_ui->m_notify->setWordWrap(true);

    connect(m_ui->m_cancel, &QPushButton::clicked, this, &DeleteNotify::close);
    connect(m_ui->m_ok, &QPushButton::clicked, this, [this]
            {
                close();
                emit accepted();
            });
}
}  // namespace KS
