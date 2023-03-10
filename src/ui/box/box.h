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

#include <QPushButton>

class BoxManagerProxy;

namespace KS
{
struct BoxInfo
{
    // uid
    QString uid;
    // 名称
    QString name;
    // 是否挂载
    bool mounted;
};

class Box : public QPushButton
{
    Q_OBJECT

public:
    Box(const QString &boxUID);
    virtual ~Box(){};

private:
    BoxManagerProxy *m_boxManagerProxy;
};

}  // namespace KS
