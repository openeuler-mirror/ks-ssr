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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/ui/navigation.h"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace KS
{
NavigationItem::NavigationItem(const QString &iconName,
                               const QString &description) : m_icon(nullptr),
                                                             m_description(nullptr)
{
    this->setFixedWidth(88);
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    this->m_icon = new QPushButton(this);
    this->m_icon->setCheckable(true);
    this->m_icon->setFlat(true);
    this->m_icon->setObjectName("m_icon");
    this->m_icon->setIcon(QIcon(iconName));
    this->m_icon->setIconSize(QSize(48, 48));
    this->m_icon->setFixedSize(QSize(64, 64));

    layout->addWidget(this->m_icon);
    layout->setAlignment(this->m_icon, Qt::AlignCenter);

    this->m_description = new QLabel(this);
    // 主要为了适配英文环境下文字显示，如果不换行可能会导致文字显示不全
    this->m_description->setWordWrap(true);
    this->m_description->setText(description);
    this->m_description->setAlignment(Qt::AlignCenter);
    layout->addWidget(this->m_description);
    layout->setAlignment(this->m_description, Qt::AlignCenter);
    layout->addStretch(0);

    this->setLayout(layout);

    connect(this->m_icon, &QPushButton::clicked, [this](bool checked)
            { this->clicked(checked); });
}

Navigation::Navigation(QWidget *parent) : QWidget(parent)
{
    this->m_buttons = new QButtonGroup(this);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(this->m_buttons, &QButtonGroup::idClicked, [this](int id)
            { Q_EMIT this->currentCategoryChanged(id); });
#else
    connect(this->m_buttons, QOverload<int>::of(&QButtonGroup::buttonClicked), [this](int id)
            { Q_EMIT this->currentCategoryChanged(id); });
#endif
}

void Navigation::addItem(NavigationItem *item)
{
    this->m_buttons->addButton(item->getButton(), this->layout()->count());
    this->layout()->addWidget(item);

    // connect(item, &NavigationItem::clicked, this, [this, item](bool checked)
    //         {
    //             auto index = this->layout()->indexOf(item);
    //             Q_EMIT this->currentCategoryChanged(index);
    //         });
}

}  // namespace KS
