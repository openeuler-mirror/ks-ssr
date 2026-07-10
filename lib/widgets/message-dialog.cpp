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
 * Author:     chendingjian <chendingjian@kylinsec.com.cn>
 */
#include "message-dialog.h"
#include <qt5-log-i.h>
#include <QEventLoop>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>
#include <QVBoxLayout>

namespace KS
{
MessageDialog::MessageDialog(QWidget *parent, bool canGetResult)
    : TitlebarWindow(parent),
      m_messageLabel(nullptr),
      m_result(false)
{
    initUI(canGetResult);
}

MessageDialog::~MessageDialog()
{
    //    KLOG_DEBUG() << "The MessageDialog is deleted.";
}

void MessageDialog::setMessage(const QString &message)
{
    m_messageLabel->setText(message);
}

bool MessageDialog::exec()
{
    show();

    QEventLoop loop;
    connect(this, &MessageDialog::finished, &loop, &QEventLoop::quit);
    loop.exec();  // Start the event loop

    return m_result;
}

void MessageDialog::initUI(bool canGetResult)
{
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    setFixedSize(259, 229);

    auto vlay = new QVBoxLayout(getWindowContentWidget());
    vlay->setContentsMargins(4, 4, 4, 4);

    auto cusWidget = new QWidget(getWindowContentWidget());
    auto contentLayout = new QVBoxLayout(cusWidget);
    cusWidget->setObjectName("messageDialog");
    contentLayout->setContentsMargins(24, 24, 24, 24);

    vlay->addWidget(cusWidget);

    setTitle(tr("Notify"));
    m_messageLabel = new QLabel(this);
    m_messageLabel->setMinimumWidth(180);
    // 自动换行
    m_messageLabel->setWordWrap(true);
    // 可复制
    m_messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    contentLayout->addWidget(m_messageLabel);
    contentLayout->addStretch();

    auto *ok = new QPushButton(tr("ok"), this);
    ok->setFixedSize(72, 36);
    ok->setProperty("okStyle", QVariant(true));
    if (!canGetResult)
    {
        // 页面关闭时销毁
        setAttribute(Qt::WA_DeleteOnClose);

        connect(ok, &QPushButton::clicked, this, &MessageDialog::close);
        contentLayout->addWidget(ok, 0, Qt::AlignHCenter);
    }
    else
    {
        auto *cancel = new QPushButton(tr("cancel"), this);
        cancel->setFixedSize(72, 36);

        connect(ok, &QPushButton::clicked, [this]()
                {
                    m_result = true;
                    this->close();
                    emit finished();
                });

        connect(cancel, &QPushButton::clicked, [this]()
                {
                    m_result = false;
                    this->close();
                    emit finished();
                });

        auto btnLayout = new QHBoxLayout(cusWidget);
        btnLayout->addWidget(ok);
        btnLayout->addWidget(cancel);
        contentLayout->addLayout(btnLayout);
    }
}

void MessageDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
}  // namespace KS
