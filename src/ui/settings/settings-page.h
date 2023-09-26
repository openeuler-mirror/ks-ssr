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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */
#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include "src/ui/common/titlebar-window.h"

namespace Ui
{
class SettingsPage;
}
namespace KS
{
class SettingsPage : public TitlebarWindow
{
    Q_OBJECT

public:
    SettingsPage(QWidget *parent = nullptr);
    ~SettingsPage();

private:
    void initUI();
    void initSidebar();
    void initSubPage();

signals:
    void trustedStatusChange(bool status);

private:
    Ui::SettingsPage *m_ui;
};

}  // namespace KS
#endif  // SETTINGSPAGE_H