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
 * Author:     chendingjian <chendingjian@kylinsec.com.cn>
 */

#pragma once

#include <QLabel>
#include <QMap>
#include "lib/widgets/window/titlebar-window.h"
#include "license/activation.h"

namespace Ui
{
class Window;
}

class DaemonProxy;
class QPushButton;

namespace KS
{
class Settings;
class Navigation;
class SideBar;
class Loading;
class PluginsManager;
class WorkPage;

namespace Accounts
{
class User;
}

class Window : public TitlebarWindow
{
    Q_OBJECT
public:
    Window();
    virtual ~Window();

    void start();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void init();
    void initNotification();
    // 窗口整体初始化
    void initWindow();
    /// 初始化标题栏
    void initTitlebar();
    void addWorkPage(WorkPage *page);
    void hideLoading(bool ishide);
    void clearSidebar();

    // 登录成功后显示窗口内容
    void initWindowContent();
    void initPages();
    void initWorkPages();
    void initSettingPages();
    void initNavigation();
    // 根据权重的导航分类和侧边分类切换页面
    void switchWorkPage();
    void switchSidebars();

private slots:
    void popupSettingsDialog();
    void popupActiveDialog();
    void popupAboutDialog();
    // 单例模式激活窗口
    void activateMetaObject();
    void setNotifyStatus(bool disabled);

    void logout(const QString &userName);
    // 重新登录
    void relogin(const QString &userName);

private:
    Ui::Window *m_ui;
    // 设置对话框
    Settings *m_settingsDialog;
    // 二维数组，第一维是NavigationCategory，第二维是Page
    QVector<QList<WorkPage *>> m_workPages;
    Activation::Activation *m_activation;
    QPushButton *m_accountButton;
    // 设置选项，若无设置页面需隐藏这个按钮
    QAction *m_settingsAction;

    Accounts::User *m_user;
    // 先放到这里，后面可以考虑放到更加合适的地方管理
    PluginsManager *m_pluginManager;
    // 页面加载动画
    Loading *m_loading;
};
}  // namespace KS
