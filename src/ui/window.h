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

#include "src/ui/common/titlebar-window.h"

namespace Ui
{
class Window;
}

namespace KS
{
class Navigation;

class Window : public TitlebarWindow
{
    Q_OBJECT
public:
    Window();
    virtual ~Window();

    static Ui::Window *instance();

private:
    // 窗口整体初始化
    void initWindow();
    // 导航和导航项初始化
    void initNavigation();

private:
    Ui::Window *m_ui;
};
}  // namespace KS
