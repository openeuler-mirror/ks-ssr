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
#include "sub-window.h"
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>

namespace KS
{
SubWindow::SubWindow(QWidget *parent) : TitlebarWindow(parent),
                                        m_contentLayout(nullptr)
{
    initUI();
}

SubWindow::~SubWindow()
{
}

QVBoxLayout *SubWindow::getContentLayout()
{
    return m_contentLayout;
}

void SubWindow::buildNotify(const QString &notify)
{
    setTitle(tr("Notify"));
    auto label = new QLabel(notify, this);
    label->setMinimumWidth(180);
    // 自动换行
    label->setWordWrap(true);
    // 可复制
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto labelHBox = new QHBoxLayout(this);
    labelHBox->addStretch();
    labelHBox->addWidget(label);
    labelHBox->addStretch();

    auto *ok = new QPushButton(tr("ok"), this);
    ok->setFixedSize(72, 36);
    ok->setObjectName("okBtn");
    connect(ok, &QPushButton::clicked, this, &QWidget::close);

    auto okHBox = new QHBoxLayout(this);
    okHBox->addStretch();
    okHBox->addWidget(ok);
    okHBox->addStretch();

    m_contentLayout->addLayout(labelHBox);
    m_contentLayout->addLayout(okHBox);
}

void SubWindow::initUI()
{
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    //    setFixedSize(300, 220);

    auto vlay = new QVBoxLayout(getWindowContentWidget());
    vlay->setContentsMargins(4, 4, 4, 4);

    auto cusWidget = new QWidget(this);
    m_contentLayout = new QVBoxLayout(cusWidget);
    cusWidget->setObjectName("cusWidget");
    m_contentLayout->setContentsMargins(12, 12, 12, 12);

    vlay->addWidget(cusWidget);
    //    show();
}

void SubWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

}  // namespace KS