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

#pragma once

#include <QString>
#include "category.h"
namespace KS
{
namespace BR
{
namespace Plugins
{
class Categories
{
public:
    Categories();

public:
    void setRow(int row);
    void setName(const QString &name);
    void setLabel(const QString &label);
    void setIconName(const QString &iconName);
    void setDescription(const QString &iconName);
    void setCategory(Category *category);

    int getRow();
    QString getName();
    QString getLabel();
    QString getIconName();
    QList<Category *> getCategory();

    Category *find(const QString &name);
    void clearState(int state);

private:
    int m_row;
    QString m_iconName;
    QString m_label;
    QString m_name;
    QString m_description;
    QList<Category *> m_category;
};

}  // namespace Plugins
}  // namespace BR
}  // namespace KS
