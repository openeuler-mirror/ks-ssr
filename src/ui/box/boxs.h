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

#pragma once

#include <QMap>
#include <QPair>
#include "src/ui/box/box.h"

namespace KS
{
class Boxs : public QWidget
{
    Q_OBJECT

public:
    Boxs(QWidget *parent = nullptr);
    virtual ~Boxs(){};

    void addBox(Box *box);
    void removeBox(Box *box);

private:
    // 记录每个保险箱的显示位置
    QMap<QString, QPair<int, int>> m_boxPosition;
    // 最后一个保险箱的位置
    int m_lastBoxRow;
    int m_lastBoxCol;
};

}  // namespace KS
