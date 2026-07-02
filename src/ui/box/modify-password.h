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

#include <QWidget>

namespace Ui
{
class ModifyPassword;
};

namespace KS
{
class ModifyPassword : public QWidget
{
    Q_OBJECT

public:
    ModifyPassword(QWidget *parent = nullptr);
    virtual ~ModifyPassword(){};

    QString getCurrentPassword();
    QString getNewPassword();

Q_SIGNALS:
    void accepted();
    void rejected();

private:
    Ui::ModifyPassword *m_ui;
};
}  // namespace KS