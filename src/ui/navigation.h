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

#pragma once

#include <QPushButton>

class QPushButton;
class QLabel;
class QButtonGroup;

namespace KS
{
class NavigationItem : public QWidget
{
    Q_OBJECT
public:
    NavigationItem(const QString &iconName, const QString &description);
    virtual ~NavigationItem(){};

    QAbstractButton *getButton() { return this->m_icon; };

Q_SIGNALS:
    void clicked(bool checked);

private:
    QPushButton *m_icon;
    QLabel *m_description;
};

class Navigation : public QWidget
{
    Q_OBJECT

public:
    Navigation(QWidget *parent = nullptr);
    virtual ~Navigation(){};

    // 导航栏添加分类项
    void addItem(NavigationItem *item);

Q_SIGNALS:
    void currentCategoryChanged(int index);

private:
    QButtonGroup *m_buttons;
};

}  // namespace KS
