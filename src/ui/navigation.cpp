/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * kiran-session-manager is licensed under Mulan PSL v2.
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
#include <QHBoxLayout>

namespace KS
{
NavigationItem::NavigationItem(const QString &iconName)
{
    this->setIcon(QIcon(iconName));
    this->setIconSize(QSize(64, 64));
}

Navigation::Navigation(QWidget *parent) : QWidget(parent)
{
    this->setLayout(new QHBoxLayout());
}

void Navigation::addItem(NavigationItem *item)
{
    this->layout()->addWidget(item);

    connect(item, &NavigationItem::click, this, [this, item]() {
        auto index = this->layout()->indexOf(item);
        Q_EMIT this->currentCategoryChanged(index);
    });
}

}  // namespace KS
