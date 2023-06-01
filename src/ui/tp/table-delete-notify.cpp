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
#include "table-delete-notify.h"
#include "ui_table-delete-notify.h"

namespace KS
{
TableDeleteNotify::TableDeleteNotify(QWidget *parent) : TitlebarWindow(parent),
                                                        m_ui(new Ui::TableDeleteNotify)
{
    m_ui->setupUi(getWindowContentWidget());

    init();
}

TableDeleteNotify::~TableDeleteNotify()
{
    delete m_ui;
}

void TableDeleteNotify::init()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    setFixedSize(299, 219);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitle(tr("Remove protection"));
    m_ui->m_notify->setText(tr("The removal operation is irreversible."
                               "Do you confirm the removal of the selected record from the whitelist?"));
    m_ui->m_notify->setWordWrap(true);

    connect(m_ui->m_cancel, &QPushButton::clicked, this, &TableDeleteNotify::close);
    connect(m_ui->m_ok, &QPushButton::clicked, this, [this]
            {
                close();
                emit accepted();
            });
}
}  // namespace KS
