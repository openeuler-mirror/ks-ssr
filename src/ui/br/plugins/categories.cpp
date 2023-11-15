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

#include "categories.h"

namespace KS
{
namespace BR
{
namespace Plugins
{
Categories::Categories()
{
}

void Categories::setRow(int row)
{
    m_row = row;
}

void Categories::setName(const QString &name)
{
    m_name = name;
}

void Categories::setLabel(const QString &label)
{
    m_label = label;
}

void Categories::setIconName(const QString &iconName)
{
    m_iconName = iconName;
}

void Categories::setDescription(const QString &description)
{
    m_description = description;
}

void Categories::setCategory(Category *category)
{
    m_category.append(category);
}

int Categories::getRow()
{
    return m_row;
}

QString Categories::getName()
{
    return m_name;
}

QString Categories::getLabel()
{
    return m_label;
}

QString Categories::getIconName()
{
    return m_iconName;
}

QList<Category *> Categories::getCategory()
{
    return m_category;
}

Category *Categories::find(const QString &name)
{
    for (auto category : m_category)
    {
        if (category->getName() == name)
            return category;
    }
    return NULL;
}

void Categories::clearState(int state)
{
    for (auto category : m_category)
    {
        category->setState(state);
    }
}
}  // namespace Plugins
}  // namespace BR
}  // namespace KS
