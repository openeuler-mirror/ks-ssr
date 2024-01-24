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

#include <QWidget>
#include "src/ui/common/window/titlebar-window.h"

namespace Ui
{
class Dialog;
}
namespace KS
{
namespace Settings
{
class Dialog : public TitlebarWindow
{
    Q_OBJECT
public:
    static void globalInit(QWidget *parent);
    static void globalDeinit();

    static Dialog *instance()
    {
        return m_instance;
    };

    void addSidebars(const QStringList &sidebarNames);
    // 获取回退状态，当退回进行中时，不允许用户退出
    uint getFallbackStatus();

private:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    void initUI();
    void initSidebar();
    void initSubPage();
    void addSubPage(const QString &sidebarName);

signals:
    void exportStrategyClicked();
    void resetAllArgsClicked();

private:
    Ui::Dialog *m_ui;
    static Dialog *m_instance;
};

}  // namespace Settings
}  // namespace KS