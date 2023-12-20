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

#include <QLabel>
#include <QMap>
#include "src/ui/common/page.h"
#include "src/ui/common/window/titlebar-window.h"
#include "src/ui/license/activation.h"

namespace Ui
{
class Window;
}

class DaemonProxy;

namespace KS
{
class Navigation;
class SideBar;
class Loading;
class LicenseProxy;
class Window : public TitlebarWindow
{
    Q_OBJECT
public:
    Window();
    virtual ~Window();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void login();
    void start();
    void initActivation();
    // 窗口整体初始化
    void initWindow();
    // 导航项以及子页面初始化
    void initPageAndNavigation();
    void initSettings();
    void addPage(Page *page);
    void showLoading(bool isShow);
    void clearSidebar();

private slots:
    void popupActiveDialog();
    void updateActivation();
    void popupSettingsDialog();
    void popupAboutDialog();
    // 单例模式激活窗口
    void activateMetaObject();
    void updatePage();
    void updateSidebar();

    void logout(const QString &userName);
    // 重新登录
    void relogin(const QString &userName);

private:
    Ui::Window *m_ui;
    QMap<QString, QList<Page *>> m_pages;
    Activation::Activation *m_activation;
    QLabel *m_activateStatus;
    Loading *m_loading;
    QSharedPointer<LicenseProxy> m_licenseProxy;
    DaemonProxy *m_dbusProxy;
};
}  // namespace KS
