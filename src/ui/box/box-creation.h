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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include "src/ui/common/titlebar-window.h"

namespace Ui
{
class BoxCreation;
};

namespace KS
{
// 创建保险箱页面
class BoxCreation : public TitlebarWindow
{
    Q_OBJECT

public:
    BoxCreation(QWidget *parent = nullptr);
    virtual ~BoxCreation(){};

    QString getName();
    QString getPassword();

private slots:
    void onOkClicked();

signals:
    void accepted();
    void rejected();
    // 两次密码不一致
    void passwdInconsistent();
    // 输入空字符
    void inputEmpty();

private:
    Ui::BoxCreation *m_ui;
};

}  // namespace KS
