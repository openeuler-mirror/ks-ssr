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
#include "custom-window.h"
#include <QIcon>
#include <QLabel>
#include <QPushButton>

namespace KS
{
CustomWindow::CustomWindow(QWidget *parent) : TitlebarWindow(parent),
                                              m_contentLayout(nullptr)
{
    this->initUI();
}

CustomWindow::~CustomWindow()
{
}

QVBoxLayout *CustomWindow::getContentLayout()
{
    return this->m_contentLayout;
}

void CustomWindow::buildNotify(const QString &notify)
{
    this->setTitle(tr("Notify"));
    auto label = new QLabel(notify);
    // 自动换行
    label->setWordWrap(true);

    auto labelHBox = new QHBoxLayout(this);
    labelHBox->addStretch();
    labelHBox->addWidget(label);
    labelHBox->addStretch();

    auto *ok = new QPushButton(tr("ok"), this);
    ok->setFixedSize(80, 36);
    ok->setStyleSheet("QPushButton{"
                      "color:#FFFFFF;"
                      "font:NotoSansCJKsc-Regular;"
                      "font-size:12px;"
                      "border-radius:8px;"
                      "background:#43A3F2;}"
                      "QPushButton:hover{"
                      "background:#79C3FF;"
                      "border:4px;}");
    connect(ok, &QPushButton::clicked, this, &QWidget::close);

    auto okHBox = new QHBoxLayout(this);
    okHBox->addStretch();
    okHBox->addWidget(ok);
    okHBox->addStretch();

    m_contentLayout->addLayout(labelHBox);
    m_contentLayout->addLayout(okHBox);
}

void CustomWindow::initUI()
{
    this->setWindowModality(Qt::ApplicationModal);
    this->setIcon(QIcon(":/images/logo"));
    this->setResizeable(false);
    this->setTitleBarHeight(36);
    this->setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    //    this->setFixedSize(300, 220);

    auto vlay = new QVBoxLayout(this->getWindowContentWidget());
    vlay->setContentsMargins(4, 4, 4, 4);

    auto cusWidget = new QWidget(this);
    m_contentLayout = new QVBoxLayout(cusWidget);
    m_contentLayout->setContentsMargins(12, 12, 12, 12);

    cusWidget->setStyleSheet("background-color: #2d2d2d;"
                             "border-radius: 6px;");
    vlay->addWidget(cusWidget);
    this->show();
}

}  // namespace KS
