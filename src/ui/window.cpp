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

#include "src/ui/window.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QFile>
#include <QMenu>
#include <QMutex>
#include <QPushButton>
#include <QStackedWidget>
#include <QX11Info>
#include "include/ssr-i.h"
#include "lib/base/notification-wrapper.h"
#include "lib/license/license-proxy.h"
#include "src/ui/about.h"
#include "src/ui/account/manager.h"
#include "src/ui/br/br-page.h"
#include "src/ui/common/loading.h"
#include "src/ui/common/single-application/single-application.h"
#include "src/ui/daemon_proxy.h"
#include "src/ui/dm/device-list-page.h"
#include "src/ui/dm/device-log-page.h"
#include "src/ui/fp/file-protection-page.h"
#include "src/ui/log/log-page.h"
#include "src/ui/navigation.h"
#include "src/ui/private-box/box-page.h"
#include "src/ui/settings/dialog.h"
#include "src/ui/sidebar.h"
#include "src/ui/tool-box/access-control/access-control-page.h"
#include "src/ui/tool-box/file-shred/file-shred-page.h"
#include "src/ui/tool-box/file-sign/file-sign-page.h"
#include "src/ui/tool-box/privacy-cleanup/privacy-cleanup-page.h"
#include "src/ui/tp/execute-protected-page.h"
#include "src/ui/tp/kernel-protected-page.h"
#include "src/ui/ui_window.h"
#include "ssr-marcos.h"

namespace KS
{
#define SSR_STYLE_PATH ":/styles/ssr"
// 检测命令是否存在
#define KSS_CMD_PATH SSR_INSTALL_BINDIR "/kss"

// 锁屏状态
#define KIRAN_SCREENSAVER_DBUS_NAME "com.kylinsec.Kiran.ScreenSaver"
#define KIRAN_SCREENSAVER_DBUS_PATH "/com/kylinsec/Kiran/ScreenSaver"
#define KIRAN_SCREENSAVER_DBUS_INTERFACE "com.kylinsec.Kiran.ScreenSaver"

#define MATE_SCREENSAVER_DBUS_NAME "org.mate.ScreenSaver"
#define MATE_SCREENSAVER_DBUS_PATH "/"
#define MATE_SCREENSAVER_DBUS_INTERFACE "org.mate.ScreenSaver"

Window::Window()
    : TitlebarWindow(nullptr),
      m_ui(new Ui::Window),
      m_activation(nullptr),
      m_activateStatus(nullptr),
      m_loading(nullptr),
      m_licenseProxy(nullptr)
{
    m_ui->setupUi(getWindowContentWidget());
    m_dbusProxy = new DaemonProxy(SSR_DBUS_NAME,
                                  SSR_DBUS_OBJECT_PATH,
                                  QDBusConnection::systemBus(),
                                  this);
    Account::Manager::globalInit(this);
    initNotification();
    initWindow();
    initActivation();
    if (m_licenseProxy->isActivated())
    {
        login();
    }
    else
    {
        connect(
            m_licenseProxy.data(), &LicenseProxy::licenseChanged, this, [this] {
                disconnect(m_licenseProxy.data(), &LicenseProxy::licenseChanged, this, nullptr);
                connect(
                    m_dbusProxy, &DaemonProxy::RegisterFinished, this, [this] {
                        disconnect(m_dbusProxy, &DaemonProxy::RegisterFinished, this, nullptr);
                        login();
                    },
                    Qt::ConnectionType::UniqueConnection);
            },
            Qt::ConnectionType::UniqueConnection);
    }

    connect(dynamic_cast<SingleApplication *>(qApp), &SingleApplication::instanceStarted, this, &Window::activateMetaObject, Qt::ConnectionType::UniqueConnection);
}

Window::~Window()
{
    delete m_ui;
    Settings::Dialog::globalDeinit();
    Account::Manager::globalDeinit();
    Notify::NotificationWrapper::globalDeinit();
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

void Window::login()
{
    connect(Account::Manager::instance(), &Account::Manager::loginFinished, this, &Window::start, Qt::ConnectionType::UniqueConnection);
    connect(
        Account::Manager::instance(), &Account::Manager::softExited, this, [] {
            qApp->quit();
        },
        Qt::ConnectionType::UniqueConnection);
    connect(Account::Manager::instance(), &Account::Manager::passwordChanged, this, &Window::relogin, Qt::ConnectionType::UniqueConnection);
    Account::Manager::instance()->showLogin();
}

void Window::start()
{
    initPageAndNavigation();
    initSettings();
}

void Window::initActivation()
{
    m_activation = new Activation::Activation(this);
    m_licenseProxy = LicenseProxy::getDefault();

    if (!m_licenseProxy->isActivated())
    {
        m_activateStatus->show();
        auto x = this->x() + this->width() / 4 + m_activation->width() / 4;
        auto y = this->y() + this->height() / 4 + m_activation->height() / 4;
        m_activation->move(x, y);
        m_activation->raise();
        m_activation->show();
    }
    connect(m_activation, &Activation::Activation::closed, [this] {
        // 未激活状态下获取关闭信号，则退出程序;已激活状态下后获取关闭信号，只是隐藏激活对话框
        if (!m_licenseProxy->isActivated())
            qApp->quit();
        else
            m_activation->hide();
    });
    connect(m_licenseProxy.data(), &LicenseProxy::licenseChanged, this, &Window::updateActivation, Qt::UniqueConnection);
}

void Window::initNotification()
{
    Notify::NotificationWrapper::globalInit(tr("Safety reinforcement").toStdString());
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
    setTitle(tr("Security reinforcement"));
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

    setTitlebarCustomLayoutAlignHCenter(false);
    auto layout = getTitlebarCustomLayout();
    layout->setContentsMargins(0, 0, 10, 0);
    layout->setSpacing(10);

    // 未激活文本
    m_activateStatus = new QLabel(this);
    m_activateStatus->setObjectName("activateStatus");
    m_activateStatus->setFixedHeight(18);
    m_activateStatus->setAlignment(Qt::AlignHCenter);
    m_activateStatus->setText(tr("Unactivated"));
    m_activateStatus->hide();

    // 创建账户管理按钮
    auto accountButton = new QPushButton(this);
    accountButton->setObjectName("accountButton");
    accountButton->setFixedSize(QSize(16, 16));

    auto accountMenu = new QMenu(this);
    accountButton->setMenu(accountMenu);

    accountMenu->addAction(tr("Modify password"), this, [] {
        Account::Manager::instance()->showPasswordModification();
    });
    accountMenu->addAction(tr("Logout"), this, [this] {
        logout(Account::Manager::instance()->getCurrentUserName());
    });

    // 创建标题栏右侧菜单按钮
    auto btnForMenu = new QPushButton(this);
    btnForMenu->setObjectName("btnForMenu");
    btnForMenu->setFixedSize(QSize(16, 16));

    auto settingMenu = new QMenu(this);
    btnForMenu->setMenu(settingMenu);

    settingMenu->addAction(tr("Settings"), this, &Window::popupSettingsDialog);
    settingMenu->addAction(tr("Activation"), this, &Window::popupActiveDialog);
    settingMenu->addAction(tr("About"), this, &Window::popupAboutDialog);

    layout->addWidget(accountButton);
    layout->addWidget(m_activateStatus);
    layout->addWidget(btnForMenu);
    layout->setAlignment(Qt::AlignRight);
}

void Window::initPageAndNavigation()
{
    // 移除qt designer默认创建的widget
    while (m_ui->m_stackedPages->currentWidget() != nullptr)
    {
        auto currentWidget = m_ui->m_stackedPages->currentWidget();
        m_ui->m_stackedPages->removeWidget(currentWidget);
        delete currentWidget;
    }
    // 页面加载动画
    m_loading = new Loading(this);
    addPage(new BR::BRPage(this));
    // TODO 暂时通过有无kss命令的方式判断是否支持可信，需考虑更好的方法
    if (QFile::exists(KSS_CMD_PATH))
    {
        // 可信保护页面需判断是否加载成功
        auto execute = new TP::ExecuteProtectedPage(this);
        connect(
            execute, &TP::ExecuteProtectedPage::initFinished, this, [this] {
                m_loading->setVisible(false);
                m_ui->m_sidebar->setEnabled(true);
                updatePage();
            },
            Qt::ConnectionType::UniqueConnection);
        addPage(execute);
        addPage(new TP::KernelProtectedPage(this));
        addPage(new FP::FileProtectionPage(this));
        m_loading->setFixedSize(execute->size());
    }
    addPage(new PrivateBox::BoxPage(this));
    addPage(new DM::DeviceListPage(this));
    // TODO 新增日志模块写入设备日志，旧的设备日志代码是否需要保留？若后续没有作用了在发布之前删除
    // addPage(new DM::DeviceLogPage(this));
    addPage(new ToolBox::FileSign(this));
    addPage(new ToolBox::FileShredPage(this));
    addPage(new ToolBox::PrivacyCleanupPage(this));
    addPage(new ToolBox::AccessControlPage(this));
    addPage(new Log::LogPage(this));
    m_ui->m_stackedPages->addWidget(m_loading);
    m_ui->m_stackedPages->setCurrentIndex(0);

    // 通过页面获取是否有对应的导航栏
    for (auto pages : m_pages.values())
    {
        auto navigationUID = pages.first()->getNavigationUID();
        CONTINUE_IF_TRUE(navigationUID.isEmpty());

        if (navigationUID == tr("Baseline reinforcement"))
        {
            m_ui->m_navigation->addItem(new NavigationItem(":/images/baseline-reinforcement", navigationUID));
        }
        else if (navigationUID == tr("Trusted protected"))
        {
            m_ui->m_navigation->addItem(new NavigationItem(":/images/trusted-protected", tr("Trusted protected")));
        }
        else if (navigationUID == tr("File protected"))
        {
            m_ui->m_navigation->addItem(new NavigationItem(":/images/file-protected", tr("File protected")));
        }
        else if (navigationUID == tr("Private box"))
        {
            m_ui->m_navigation->addItem(new NavigationItem(":/images/box-manager", tr("Private box")));
        }
        else if (navigationUID == tr("Device management"))
        {
            m_ui->m_navigation->addItem(new NavigationItem(":/images/device", tr("Device management")));
        }
        else if (navigationUID == tr("Tool Box"))
        {
            m_ui->m_navigation->addItem(new NavigationItem(":/images/tool-box", tr("Tool Box")));
        }
        else if (navigationUID == tr("Log audit"))
        {
            m_ui->m_navigation->addItem(new NavigationItem(":/images/log-audit", tr("Log audit")));
        }
    }
    m_ui->m_navigation->setBtnChecked(0);

    connect(m_ui->m_navigation, SIGNAL(currentUIDChanged()), this, SLOT(updatePage()), Qt::ConnectionType::UniqueConnection);
    connect(m_ui->m_sidebar, &SideBar::itemChanged, this, &Window::updateSidebar, Qt::UniqueConnection);

    updatePage();
}

void Window::initSettings()
{
    Settings::Dialog::globalInit(this);
    QStringList settingsSidebars;
    // 通过登入账户判断需要显示的设置页面
    auto currentUser = Account::Manager::instance()->getCurrentUserName();
    if (currentUser == SSR_ACCOUNT_NAME_SYSADM)
    {
        settingsSidebars << tr("Baseline reinforcement") << tr("Interface Control");
    }
    else if (currentUser == SSR_ACCOUNT_NAME_SECADM)
    {
        settingsSidebars << tr("Trusted protect") << tr("Identity authentication");
    }
    else if (currentUser == SSR_ACCOUNT_NAME_AUDADM)
    {
        // TODO audit用户暂无设置
    }
    Settings::Dialog::instance()->addSidebars(settingsSidebars);
    // 导出策略需要从表格中获取勾选项，设置页面中无法获取，通过信号实现
    connect(
        Settings::Dialog::instance(), &Settings::Dialog::exportStrategyClicked, this, [this] {
            for (auto page : m_pages.value(tr("Baseline reinforcement")))
            {
                if (page->isVisible())
                {
                    auto brPage = static_cast<BR::BRPage *>(page);
                    brPage->exportStrategy();
                }
            }
        },
        Qt::UniqueConnection);
    connect(
        Settings::Dialog::instance(), &Settings::Dialog::resetAllArgsClicked, this, [this] {
            for (auto page : m_pages.value(tr("Baseline reinforcement")))
            {
                if (page->isVisible())
                {
                    auto brPage = static_cast<BR::BRPage *>(page);
                    brPage->resetAllReinforcementArgs();
                }
            }
        },
        Qt::UniqueConnection);
}

void Window::addPage(Page *page)
{
    // 通过用户权限添加页面
    RETURN_IF_TRUE(page->getAccountRoleName() != Account::Manager::instance()->getCurrentUserName() && page->getAccountRoleName() != SSR_ACCOUNT_NAME_COMADM);
    if (!m_pages.contains(page->getNavigationUID()))
    {
        QList<Page *> pages;
        pages.append(page);

        m_pages.insert(page->getNavigationUID(), pages);
    }
    else
    {
        m_pages.find(page->getNavigationUID()).value().append(page);
    }

    m_ui->m_stackedPages->addWidget(page);
}

void Window::showLoading(bool isShow)
{
    RETURN_IF_TRUE(isShow)

    if (!m_loading->isVisible())
    {
        m_loading->setVisible(true);
    }

    m_ui->m_stackedPages->setCurrentWidget(m_loading);
    m_ui->m_sidebar->setEnabled(false);
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

void Window::popupActiveDialog()
{
    m_activation->update();
    auto x = this->x() + this->width() / 4 + m_activation->width() / 16;
    auto y = this->y() + this->height() / 4 + m_activation->height() / 16;
    m_activation->move(x, y);
    m_activation->show();
}

void Window::updateActivation()
{
    bool isActivate = m_licenseProxy->isActivated();
    // 设置激活对话框和激活状态标签是否可见
    m_activation->setVisible(!isActivate);
    m_activateStatus->setVisible(!isActivate);
}

void Window::popupSettingsDialog()
{
    auto x = this->x() / 4 + this->width() / 4 + Settings::Dialog::instance()->width() / 16;
    auto y = this->y() / 4 + this->height() / 4 + Settings::Dialog::instance()->height() / 16;
    Settings::Dialog::instance()->move(x, y);
    Settings::Dialog::instance()->show();
}

void Window::popupAboutDialog()
{
    auto aboutDialog = new About(this);

    auto x = this->x() + this->width() / 4 + aboutDialog->width() / 16;
    auto y = this->y() + this->height() / 4 + aboutDialog->height() / 16;
    aboutDialog->move(x, y);
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
    showNormal();
    raise();
    activateWindow();
}

void Window::updatePage()
{
    // 清空侧边栏
    clearSidebar();
    // 插入侧边栏
    auto pages = m_pages.find(m_ui->m_navigation->getSelectedUID()).value();
    RETURN_IF_TRUE(pages.count() == 0)
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
    m_ui->m_stackedPages->setCurrentWidget(pages.first());

    // 没有分侧边栏则隐藏
    if (m_ui->m_sidebar->count() == 0)
    {
        m_ui->m_sidebar->hide();
    }
    else
    {
        m_ui->m_sidebar->show();
    }

    // 可信页面需要检测是否加载成功
    if (tr("Trusted protected") == pages.first()->getNavigationUID())
    {
        auto page = qobject_cast<TP::ExecuteProtectedPage *>(pages.first());
        showLoading(page->getInitialized());
    }
}

void Window::updateSidebar()
{
    RETURN_IF_TRUE(m_ui->m_sidebar->count() == 0)

    auto pages = m_pages.find(m_ui->m_navigation->getSelectedUID()).value();
    RETURN_IF_TRUE(pages.count() == 0)

    for (auto page : pages)
    {
        if (page->getSidebarUID() == m_ui->m_sidebar->getSelectedUID())
        {
            // 更新页面
            m_ui->m_stackedPages->setCurrentWidget(page);
            break;
        }
    }
}

void Window::setNotifyStatus(bool disabled)
{
    Notify::NotificationWrapper::getInstance()->setNofityEnable(!disabled);
}

void Window::logout(const QString &userName)
{
    clearSidebar();
    while (m_ui->m_stackedPages->currentWidget() != nullptr)
    {
        auto currentWidget = m_ui->m_stackedPages->currentWidget();
        m_ui->m_stackedPages->removeWidget(currentWidget);
        delete currentWidget;
    }
    m_pages.clear();
    m_ui->m_navigation->clearItems();

    Account::Manager::instance()->setLoginUserName(userName);
    Account::Manager::instance()->logout();
}

void Window::relogin(const QString &userName)
{
    if (userName == Account::Manager::instance()->getCurrentUserName())
    {
        logout(userName);
    }
}
}  // namespace KS
