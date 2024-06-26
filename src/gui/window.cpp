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

#include "window.h"
#include <page.h>
#include <qt5-log-i.h>
#include <ui-plugin-i.h>
#include <QCloseEvent>
#include <QDBusConnection>
#include <QDesktopServices>
#include <QFile>
#include <QMenu>
#include <QMutex>
#include <QPushButton>
#include <QStackedWidget>
#include <QX11Info>
#include "about.h"
#include "account_proxy.h"
#include "accounts/user-entity.h"
#include "accounts/user-fake.h"
#include "config.h"
#include "include/ssr-i.h"
#include "lib/base/notification-wrapper.h"
#include "lib/dbus/license-proxy.h"
#include "lib/widgets/single-application/single-application.h"
#include "lib/widgets/ssr-marcos-ui.h"
#include "loading.h"
#include "navigation.h"
#include "plugins-manager.h"
#include "settings.h"
#include "sidebar.h"
#include "ui_window.h"

namespace KS
{
#define SSR_STYLE_PATH ":/styles/ssr"
// 检测命令是否存在
#define KSS_CMD_PATH SSR_INSTALL_BINDIR "/kss"
// 帮助手册路径
#define HELP_MANUAL_PATH SSR_INSTALL_DATADIR "/help.pdf"

// 锁屏状态
#define KIRAN_SCREENSAVER_DBUS_NAME "com.kylinsec.Kiran.ScreenSaver"
#define KIRAN_SCREENSAVER_DBUS_PATH "/com/kylinsec/Kiran/ScreenSaver"
#define KIRAN_SCREENSAVER_DBUS_INTERFACE "com.kylinsec.Kiran.ScreenSaver"

#define MATE_SCREENSAVER_DBUS_NAME "org.mate.ScreenSaver"
#define MATE_SCREENSAVER_DBUS_PATH "/"
#define MATE_SCREENSAVER_DBUS_INTERFACE "org.mate.ScreenSaver"

#define REINFORCEMENT_BATCH_PATH "/usr/libexec/ssr-distribution-actuator"

Window::Window()
    : TitlebarWindow(nullptr),
      m_ui(new Ui::Window),
      m_windowContentInited(false),
      m_loading(nullptr)
{
    m_ui->setupUi(getWindowContentWidget());

    m_settingsDialog = new Settings(this);
    m_workPages.resize(int(NavigationIndex::COUNT));
    m_activation = new Activation::Activation(this);
#ifdef ENABLE_ACCOUNTS_MANAGER
    m_user = new UserEntity(this);
#else
    m_user = new UserFake(this);
#endif
    m_pluginManager = new PluginsManager(this);
    m_licenseProxy = LicenseProxy::getDefault();

    // 只能放到这里来弹框，因为激活成功后激活窗口会隐藏，如果消息框作为激活窗口的子窗口，则会导致整个程序退出
    connect(m_activation, &Activation::Activation::activated, [this](const QString &message)
            {
                POPUP_MESSAGE_DIALOG(message);
            });

    connect(m_user, &User::loginFinished, this, &Window::processActivation);
    connect(
        m_user, &User::softExited, this, []
        {
            qApp->quit();
        });
    connect(m_user, &User::passwordChanged, this, &Window::clearWindowContent);

    connect(dynamic_cast<SingleApplication *>(qApp), &SingleApplication::instanceStarted, this, &Window::activateMetaObject, Qt::ConnectionType::UniqueConnection);

    init();
}

Window::~Window()
{
    delete m_ui;
    Notify::NotificationWrapper::globalDeinit();
}

void Window::start()
{
    m_user->showLogin();
}

void Window::resizeEvent(QResizeEvent *event)
{
    Q_ASSERT(event);
    if (m_loading)
    {
        m_loading->setAutoFillBackground(true);
        m_loading->setFixedSize(720, 408);
    }
}

void Window::closeEvent(QCloseEvent *event)
{
    for (auto &pages : m_workPages)
    {
        for (auto &page : pages)
        {
            if (!page->checkCanExit())
            {
                // 弹窗阻止退出
                auto messageDialog = new KS::MessageDialog(this, true);
                messageDialog->setMessage(tr("Closing the software will interrupt the ongoing task. Are you sure to shut down?"));
                adjustWidgetPosition(messageDialog);
                bool ret = messageDialog->exec();
                if (ret)
                {
                    // 终止任务
                    if (!stopPageTask())
                    {
                        event->ignore();
                        return;
                    }
                }
                else
                {
                    event->ignore();
                    return;
                }
            }
        }
    }

    TitlebarWindow::closeEvent(event);
}

void Window::init()
{
    m_user->init();
    m_pluginManager->init();

    initNotification();
    initWindow();
}

void Window::initWindowContent()
{
    if (m_windowContentInited)
    {
        KLOG_INFO() << "The window content is already init.";
        return;
    }

    disconnect(m_licenseProxy.data(), &KS::LicenseProxy::activated, this, &Window::initWindowContent);

    m_accountButton->setToolTip(m_user->getCurrentUserName());
    initPages();
    initNavigation();
    switchSidebars();
    show();
    m_windowContentInited = true;

    connect(m_ui->m_navigation, &Navigation::currentUIDChanged, this, &Window::switchSidebars);
    connect(m_ui->m_sidebar, &SideBar::itemChanged, this, &Window::switchWorkPage);
}

void Window::initPages()
{
    initWorkPages();
    initSettingPages();
}

void Window::initWorkPages()
{
    auto availableWorkPages = m_pluginManager->createAvailableWorkPages();
    for (auto workPage : availableWorkPages)
    {
        workPage->setParent(this);
        addWorkPage(workPage);
    }

    m_loading = new Loading(this);
    m_ui->m_stackedPages->addWidget(m_loading);
    m_ui->m_stackedPages->setCurrentIndex(0);
}

void Window::initSettingPages()
{
    auto availableSettingPages = m_pluginManager->createAvailableSettingPages();
    m_settingsDialog->setSettingPages(availableSettingPages);
    // 如果没有设置页面，则隐藏设置按钮
    m_settingsAction->setVisible((availableSettingPages.size() > 0));
    // TODO: 放到插件实现
    // 导出策略需要从表格中获取勾选项，设置页面中无法获取，通过信号实现
    // connect(
    //     Settings::Dialog::instance(), &Settings::Dialog::exportStrategyClicked, this, [this]
    //     {
    //         for (auto page : m_pages.value(tr("Baseline reinforcement")))
    //         {
    //             if (page->isVisible())
    //             {
    //                 auto brPage = static_cast<BR::BRPage *>(page);
    //                 brPage->exportStrategy();
    //             }
    //         }
    //     },
    //     Qt::UniqueConnection);
    // connect(
    //     Settings::Dialog::instance(), &Settings::Dialog::resetAllArgsClicked, this, [this]
    //     {
    //         for (auto page : m_pages.value(tr("Baseline reinforcement")))
    //         {
    //             if (page->isVisible())
    //             {
    //                 auto brPage = static_cast<BR::BRPage *>(page);
    //                 brPage->resetAllReinforcementArgs();
    //             }
    //         }
    //     },
    //     Qt::UniqueConnection);
}

void Window::initNavigation()
{
    QVector<NavigationIndex> showIndexs;
    for (int i = 0; i < m_workPages.size(); ++i)
    {
        if (m_workPages[i].size() > 0)
        {
            showIndexs.push_back(NavigationIndex(i));
        }
    }

    KLOG_INFO() << "Show navigation item contains" << showIndexs;

    if (showIndexs.size() > 0)
    {
        m_ui->m_navigation->setItems(showIndexs);
        m_ui->m_navigation->setBtnChecked(0);
    }
}

void Window::initNotification()
{
    Notify::NotificationWrapper::globalInit(tr("Security Reinforcement").toStdString());
    QDBusConnection::sessionBus().connect(QString(),
                                          KIRAN_SCREENSAVER_DBUS_PATH,
                                          KIRAN_SCREENSAVER_DBUS_INTERFACE,
                                          "ActiveChanged",
                                          this,
                                          SLOT(setNotifyStatus(bool)));
    QDBusConnection::sessionBus().connect(QString(),
                                          MATE_SCREENSAVER_DBUS_PATH,
                                          MATE_SCREENSAVER_DBUS_INTERFACE,
                                          "ActiveChanged",
                                          this,
                                          SLOT(setNotifyStatus(bool)));
}

void Window::initWindow()
{
    setTitle(tr("KylinSec Security Reinforcement"));
    setIcon(QIcon(":/images/logo"));
    setFixedSize(1003, 667);
    setResizeable(false);

    // 初始化样式表
    QFile file(SSR_STYLE_PATH);
    if (file.open(QIODevice::ReadOnly))
    {
        QString windowStyle = file.readAll();
        setStyleSheet(styleSheet() + windowStyle);
    }
    else
    {
        KLOG_WARNING() << "Failed to open file " << SSR_STYLE_PATH;
    }

    initTitlebar();
}

void Window::initTitlebar()
{
    setTitlebarCustomLayoutAlignHCenter(false);
    auto layout = getTitlebarCustomLayout();
    layout->setContentsMargins(0, 0, 10, 0);
    layout->setSpacing(10);

    // 创建账户管理按钮
    m_accountButton = new QPushButton(this);
#ifdef ENABLE_ACCOUNTS_MANAGER
    m_accountButton->setObjectName("accountButton");
    m_accountButton->setFixedSize(QSize(16, 16));

    auto accountMenu = new QMenu(this);
    m_accountButton->setMenu(accountMenu);

    accountMenu->addAction(tr("Modify password"), this, [this]
                           {
                               m_user->showPasswordModification();
                           });
    accountMenu->addAction(tr("Logout"), this, [this]
                           {
                               clearWindowContent();
                               m_user->logout();
                           });
    layout->addWidget(m_accountButton);
#else
    m_accountButton->hide();
#endif

    // 创建标题栏右侧菜单按钮
    auto btnForMenu = new QPushButton(this);
    btnForMenu->setObjectName("btnForMenu");
    btnForMenu->setFixedSize(QSize(16, 16));

    auto settingMenu = new QMenu(this);
    btnForMenu->setMenu(settingMenu);

    m_settingsAction = new QAction(tr("Settings"), this);
    connect(m_settingsAction, &QAction::triggered, this, &Window::popupSettingsDialog, Qt::UniqueConnection);
    settingMenu->addAction(m_settingsAction);
    settingMenu->addAction(tr("Activation"), this, &Window::popupActivationDialog);
    settingMenu->addAction(tr("Help"), this, []
                           {
                               if (QFile::exists(HELP_MANUAL_PATH))
                               {
                                   KLOG_DEBUG() << "Open help manual PDF.";
                                   QDesktopServices::openUrl(QUrl::fromLocalFile(HELP_MANUAL_PATH));
                               }
                           });
    settingMenu->addAction(tr("About"), this, &Window::popupAboutDialog);

    layout->addWidget(btnForMenu);
    layout->setAlignment(Qt::AlignRight);
}

void Window::addWorkPage(WorkPage *workPage)
{
    m_workPages[int(workPage->getNavigationIndex())].append(workPage);
    m_ui->m_stackedPages->addWidget(workPage);
}

void Window::hideLoading(bool ishide)
{
    RETURN_IF_TRUE(ishide)

    if (!m_loading->isVisible())
    {
        m_loading->setVisible(true);
    }

    m_ui->m_stackedPages->setCurrentWidget(m_loading);
    m_ui->m_sidebar->setEnabled(false);
}

void Window::switchWorkPage()
{
    RETURN_IF_TRUE(m_ui->m_sidebar->count() == 0)

    auto selectedIndex = m_ui->m_navigation->getSelectedIndex();
    if (selectedIndex >= NavigationIndex::COUNT)
    {
        KLOG_WARNING() << "The navigation index exceed limit.";
        return;
    }

    auto workPages = m_workPages[selectedIndex];
    RETURN_IF_TRUE(workPages.size() == 0)

    WorkPage *matchWorkPage = nullptr;

    for (auto workPage : workPages)
    {
        if (workPage->getSidebarUID() == m_ui->m_sidebar->getSelectedUID())
        {
            matchWorkPage = workPage;
            break;
        }
    }

    if (!matchWorkPage)
    {
        KLOG_WARNING() << "Switch page failed, not found match page for sidebar" << m_ui->m_sidebar->getSelectedUID();
        return;
    }

    if (!matchWorkPage->isInitialized())
    {
        m_ui->m_stackedPages->setCurrentWidget(m_loading);
        connect(matchWorkPage, &WorkPage::initFinished, this, &Window::switchWorkPage);
    }
    else
    {
        m_ui->m_stackedPages->setCurrentWidget(matchWorkPage);
    }
}

void Window::switchSidebars()
{
    // 清空侧边栏
    clearSidebar();

    // 插入侧边栏
    auto selectedIndex = m_ui->m_navigation->getSelectedIndex();
    if (selectedIndex >= NavigationIndex::COUNT)
    {
        KLOG_WARNING() << "The navigation index exceed limit.";
        return;
    }

    auto pages = m_workPages[selectedIndex];
    if (pages.size() == 0)
    {
        KLOG_WARNING() << "The selected navigation";
        return;
    }

    for (auto page : pages)
    {
        auto sidebarUID = page->getSidebarUID();
        if (sidebarUID != "")
        {
            SidebarItem::ItemInfo itemInfo;
            itemInfo.name = page->getSidebarUID();
            itemInfo.icon = page->getSidebarIcon();
            m_ui->m_sidebar->addSideBarItem(new SidebarItem(itemInfo, m_ui->m_sidebar));
        }
    }
    // 更新页面 切换到第一个侧边栏
    m_ui->m_sidebar->setCurrentRow(0);
    m_ui->m_stackedPages->setCurrentWidget(pages.at(0));

    // 没有分侧边栏则隐藏
    if (m_ui->m_sidebar->count() == 0)
    {
        m_ui->m_sidebar->hide();
    }
    else
    {
        m_ui->m_sidebar->show();
    }

    // 因为侧边栏更新了，所以页面也要刷新
    switchWorkPage();

    // TODO: 这部分应该放到插件里面处理
    // 可信页面需要检测是否加载成功
    // if (tr("Trusted protected") == pages->first()->getNavigationUID())
    // {
    //     auto page = qobject_cast<TP::ExecuteProtectedPage *>(pages->first());
    //     hideLoading(page->getInitialized());
    // }
    // else
    // {
    //     // 其它侧边栏可用
    //     hideLoading(true);
    //     m_ui->m_sidebar->setEnabled(true);
    // }
}

void Window::clearWindowContent()
{
    clearSidebar();
    clearNavigation();
    clearPage();
    m_accountButton->setToolTip(QString());
    hide();

    m_windowContentInited = false;

    disconnect(m_ui->m_sidebar, &SideBar::itemChanged, this, &Window::switchWorkPage);
    disconnect(m_ui->m_navigation, &Navigation::currentUIDChanged, this, &Window::switchSidebars);
}

void Window::clearSidebar()
{
    auto count = m_ui->m_sidebar->count();
    for (auto i = 0; i < count; i++)
    {
        auto item = m_ui->m_sidebar->takeItem(0);
        delete item;
    }
}

void Window::clearNavigation()
{
    m_ui->m_navigation->clearItems();
}

void Window::clearPage()
{
    clearWorkPage();
    m_settingsDialog->clearSettingPages();
}

void Window::clearWorkPage()
{
    while (m_ui->m_stackedPages->currentWidget() != nullptr)
    {
        auto currentWidget = m_ui->m_stackedPages->currentWidget();
        m_ui->m_stackedPages->removeWidget(currentWidget);
        delete currentWidget;
    }

    for (auto i = 0; i < m_workPages.size(); ++i)
    {
        m_workPages[i].clear();
    }
}

void Window::processActivation()
{
    if (m_licenseProxy->isActivated())
    {
        initWindowContent();
    }
    else
    {
        popupActivationDialog();
        connect(m_licenseProxy.data(), &KS::LicenseProxy::activated, this, &Window::initWindowContent);
    }
}

bool Window::stopPageTask()
{
    for (auto &pages : m_workPages)
    {
        for (auto &page : pages)
        {
            if (!page->stopTask())
            {
                return false;
            }
        }
    }

    return true;
}

void Window::adjustWidgetPosition(QWidget *widget)
{
    auto x = this->x() + this->width() / 2 - widget->width() / 2;
    auto y = this->y() + this->height() / 2 - widget->height() / 2;
    widget->move(x, y);
}

void Window::popupSettingsDialog()
{
    adjustWidgetPosition(m_settingsDialog);
    m_settingsDialog->show();
}

void Window::popupActivationDialog()
{
    adjustWidgetPosition(m_settingsDialog);
    m_activation->show();
}

void Window::popupAboutDialog()
{
    auto aboutDialog = new About(this);

    adjustWidgetPosition(m_settingsDialog);
    aboutDialog->show();
}

void Window::activateMetaObject()
{
    /*
     *由于QXcbWindow::requestActivateWindow
     *之中对root窗口发送_NET_ACTIVE_WINDOW的事件之中的时间戳未更新,
     *导致窗口管理器接收时事件戳较为落后，未被正确处理
     *暂时处理办法，手动更新下X11时间，避免事件戳落后
     */

    if (windowState() & Qt::WindowMinimized)
    {
        setWindowState(windowState() & ~Qt::WindowMinimized);
    }

    QX11Info::setAppTime(QX11Info::getTimestamp());
    // 如果没有登录，则弹出登录窗口
    if (m_user->getCurrentUserName().isEmpty())
    {
        m_user->showLogin();
        return;
    }
    showNormal();
    raise();
    activateWindow();
}

void Window::setNotifyStatus(bool disabled)
{
    Notify::NotificationWrapper::getInstance()->setNofityEnable(!disabled);
}

}  // namespace KS
