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
    setFixedWidth(88);
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    m_icon = new QPushButton(this);
    m_icon->setCheckable(true);
    m_icon->setFlat(true);
    m_icon->setObjectName("m_icon");
    m_icon->setIcon(QIcon(iconName));
    m_icon->setIconSize(QSize(48, 48));
    m_icon->setFixedSize(QSize(64, 64));

    layout->addWidget(m_icon);
    layout->setAlignment(m_icon, Qt::AlignCenter);

    m_description = new QLabel(this);
    // 主要为了适配英文环境下文字显示，如果不换行可能会导致文字显示不全
    m_description->setWordWrap(true);
    m_description->setText(description);
    m_description->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_description);
    layout->setAlignment(m_description, Qt::AlignCenter);
    layout->addStretch(0);

    setLayout(layout);

    connect(m_icon, &QPushButton::clicked, [this](bool checked)
            { clicked(checked); });
}

Navigation::Navigation(QWidget *parent) : QWidget(parent)
{
    m_items = new QButtonGroup(this);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(m_items, &QButtonGroup::idClicked, [this](int id)
            { Q_EMIT currentUIDChanged(); });
#else
    connect(m_items, QOverload<int>::of(&QButtonGroup::buttonClicked), [this](int id)
            { Q_EMIT currentUIDChanged(); });
#endif
}

void Navigation::addItem(NavigationItem *item)
{
    m_items->addButton(item->getButton(), layout()->count());
    layout()->addWidget(item);
    m_itemUIDs.insert(layout()->count() - 1, item->getDescription());
}

QString Navigation::getSelectedUID()
{
    for (auto itemKey : m_itemUIDs.keys())
    {
        if (!m_items->button(itemKey)->isChecked())
            continue;
        return m_itemUIDs.value(itemKey);
    }
    return "";
}

void Navigation::setBtnChecked(int id)
{
    m_items->button(id)->setChecked(true);
}

}  // namespace KS
