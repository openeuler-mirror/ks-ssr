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
#include "item-proxy.h"
#include <QHBoxLayout>
#include <QIcon>

namespace KS
{
ItemProxy::ItemProxy(const QString &text,
                     const QString &icon,
                     int type,
                     QWidget *parent) : QWidget(parent),
                                        m_text(text),
                                        m_type(type)
{
    this->initUI(icon);
}

void ItemProxy::showArrow(bool isShow)
{
    if (isShow)
    {
        m_rightIcon->show();
    }
    else
    {
        m_rightIcon->hide();
    }
}

void ItemProxy::initUI(const QString &icon)
{
    this->setFixedSize(166, 50);
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 0, 6);

    auto *leftIcon = new QLabel(this);
    leftIcon->setFixedSize(16, 16);
    leftIcon->setPixmap(QIcon(icon).pixmap(16, 16));

    auto *text = new QLabel(this);
    text->setText(m_text);

    m_rightIcon = new QLabel(this);
    m_rightIcon->setFixedSize(16, 16);
    m_rightIcon->setPixmap(QIcon(":/images/right-arrow").pixmap(8, 10));
    m_rightIcon->hide();

    mainLayout->addWidget(leftIcon);
    mainLayout->addWidget(text);

    mainLayout->addStretch();
    mainLayout->addWidget(m_rightIcon);
}
}  // namespace KS
