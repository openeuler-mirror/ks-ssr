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

#include "category.h"

namespace KS
{
namespace BR
{
Category::Category()
{
}

void Category::setRow(int row)
{
    m_row = row;
}

void Category::setName(const QString &name)
{
    m_name = name;
}

void Category::setLabel(const QString &label)
{
    m_label = label;
}

void Category::setIconName(const QString &iconName)
{
    m_iconName = iconName;
}

void Category::setDescription(const QString &description)
{
    m_description = description;
}

void Category::setReinforcementItem(ReinforcementItem *reinforcementItem)
{
    m_reinforcementItems.append(reinforcementItem);
}

int Category::getRow()
{
    return m_row;
}

QString Category::getName()
{
    return m_name;
}

QString Category::getLabel()
{
    return m_label;
}

QString Category::getIconName()
{
    return m_iconName;
}

QList<ReinforcementItem *> Category::getReinforcementItem()
{
    return m_reinforcementItems;
}

ReinforcementItem *Category::find(const QString &name)
{
    for (auto reinforcementItem : m_reinforcementItems)
    {
        if (reinforcementItem->getName() == name)
            return reinforcementItem;
    }
    return nullptr;
}

void Category::clearState(int state)
{
    for (auto reinforcementItem : m_reinforcementItems)
    {
        reinforcementItem->setState(state);
    }
}
}  // namespace BR
}  // namespace KS
