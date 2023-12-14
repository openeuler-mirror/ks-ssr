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

#include "header-button-delegate.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>

namespace KS
{
HeaderButtonDelegate::HeaderButtonDelegate(QWidget *parent) : QPushButton(parent),
                                                              m_text(nullptr),
                                                              m_icon(nullptr),
                                                              m_layout(nullptr),
                                                              m_menu(nullptr)
{
    initUI();
}

void HeaderButtonDelegate::addMenuActions(QList<QAction *> actions)
{
    for (auto action : actions)
    {
        action->setCheckable(true);
        action->setChecked(true);
        m_menu->addAction(action);
    }
}

void HeaderButtonDelegate::setButtonText(const QString &text)
{
    m_text->setText(text);
}

QList<QAction *> HeaderButtonDelegate::getMenuActions() const
{
    return m_menu->actions();
}

void HeaderButtonDelegate::initUI()
{
    m_layout = new QHBoxLayout(this);
    m_text = new QLabel(this);
    m_icon = new QPushButton(this);
    m_layout->addWidget(m_text);
    m_icon->setIcon(QIcon(":/images/combobox-arrow"));
    m_icon->setIconSize(QSize(8, 4));
    m_layout->addWidget(m_icon);
    m_layout->addStretch();
    m_layout->setContentsMargins(16, 0, 0, 0);

    m_menu = new QMenu(this);
    setMenu(m_menu);
    connect(m_menu, &QMenu::triggered, this, &HeaderButtonDelegate::menuTriggered);
}
}  // namespace KS
