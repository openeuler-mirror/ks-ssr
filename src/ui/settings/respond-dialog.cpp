/**
 * @Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
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
#include "respond-dialog.h"
#include <kiran-log/qt5-log-i.h>
#include <QCloseEvent>
#include <QPushButton>
#include <QVBoxLayout>

namespace KS
{
namespace Settings
{
RespondDialog::RespondDialog(QWidget *parent) : TitlebarWindow(parent),
                                                m_message(nullptr),
                                                m_isAccepted(false)
{
    initUI();
}

RespondDialog::~RespondDialog()
{
}

void RespondDialog::setMessage(const QString &text)
{
    m_message->setText(text);
}

void RespondDialog::closeEvent(QCloseEvent *event)
{
    if (m_isAccepted)
    {
        emit accepted();
    }
    else
    {
        emit rejected();
    }

    m_isAccepted = false;
    event->ignore();
    hide();
}

void RespondDialog::initUI()
{
    setObjectName("MessageWindow");
    setTitle(tr("Notify"));
    setIcon(QIcon(":/images/logo"));
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    setResizeable(false);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setTitleBarHeight(36);
    setFixedSize(259, 219);

    auto vlay = new QVBoxLayout(getWindowContentWidget());
    vlay->setContentsMargins(4, 4, 4, 4);

    auto cusWidget = new QWidget(getWindowContentWidget());
    cusWidget->setObjectName("customWidget");
    auto contentLayout = new QVBoxLayout(cusWidget);
    contentLayout->setContentsMargins(24, 24, 24, 12);
    contentLayout->setSpacing(10);

    m_message = new QLabel(cusWidget);
    m_message->setWordWrap(true);

    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);
    auto confirm = new QPushButton(tr("Confirm"), cusWidget);
    confirm->setObjectName("confirm");
    confirm->setFixedSize(72, 36);
    auto cancel = new QPushButton(tr("Cancel"), cusWidget);
    cancel->setObjectName("cancel");
    cancel->setFixedSize(72, 36);
    btnLayout->addWidget(confirm);
    btnLayout->addWidget(cancel);

    contentLayout->addWidget(m_message);
    contentLayout->addStretch();
    contentLayout->addLayout(btnLayout);

    connect(confirm, &QPushButton::clicked,
            [=]
            {
                m_isAccepted = true;
                close();
            });
    connect(cancel, &QPushButton::clicked,
            [=]
            {
                m_isAccepted = false;
                close();
            });
    vlay->addWidget(cusWidget);
}
}  // namespace Settings
}  // namespace KS
