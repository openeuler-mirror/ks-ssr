/**
 * @Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
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
#pragma once

#include <QWidget>

class Page : public QWidget
{
    Q_OBJECT

public:
    Page(QWidget *parent = nullptr);
    virtual ~Page(){};

    virtual QString getNavigationUID() = 0;
    virtual QString getSidebarUID() = 0;
    virtual QString getSidebarIcon() = 0;
    virtual int getSelinuxType() = 0;
};
