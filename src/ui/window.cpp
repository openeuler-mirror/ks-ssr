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

#include "src/ui/window.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QFile>
#include <QMenu>
#include <QMutex>
#include <QPushButton>
#include <QStackedWidget>
#include <QX11Info>
#include "lib/license/license-proxy.h"
#include "src/ui/about.h"
#include "src/ui/br/br-page.h"
#include "src/ui/common/loading.h"
#include "src/ui/common/single-application/single-application.h"
#include "src/ui/dm/device-list-page.h"
#include "src/ui/dm/device-log-page.h"
#include "src/ui/fp/file-protection-page.h"
#include "src/ui/navigation.h"
#include "src/ui/private-box/box-page.h"
#include "src/ui/settings/dialog.h"
#include "src/ui/sidebar.h"
#include "src/ui/tp/execute-protected-page.h"
#include "src/ui/tp/kernel-protected-page.h"
#include "src/ui/ui_window.h"
#include "ssr-marcos.h"

namespace KS
{
#define SSR_STYLE_PATH ":/styles/ssr"

Window::Window() : TitlebarWindow(nullptr),
                   m_ui(new Ui::Window),
                   m_activation(nullptr),
                   m_activateStatus(nullptr),
                   m_loading(nullptr),
                   m_licenseProxy(nullptr)
{
    m_ui->setupUi(getWindowContentWidget());
    initWindow();
    initActivation();
    if (m_licenseProxy->isActivated())
    {
        start();
    }
    else
    {
        connect(m_licenseProxy.data(), &LicenseProxy::licenseChanged, this, &Window::start);
    }

    connect(dynamic_cast<SingleApplication *>(qApp), &SingleApplication::instanceStarted, this, &Window::activateMetaObject);
}

Window::~Window()
{
    delete m_ui;
    Settings::Dialog::globalDeinit();
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

void Window::start()
{
    disconnect(m_licenseProxy.data(), &LicenseProxy::licenseChanged, this, &Window::start);
    initSettings();
    initNavigation();
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
    connect(m_activation, &Activation::Activation::closed,
            [this] {
                //未激活状态下获取关闭信号，则退出程序;已激活状态下后获取关闭信号，只是隐藏激活对话框
                if (!m_licenseProxy->isActivated())
                    qApp->quit();
                else
                    m_activation->hide();
            });
    connect(m_licenseProxy.data(), &LicenseProxy::licenseChanged, this, &Window::updateActivation, Qt::UniqueConnection);
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

    //未激活文本
    m_activateStatus = new QLabel(this);
    m_activateStatus->setObjectName("activateStatus");
    m_activateStatus->setFixedHeight(18);
    m_activateStatus->setAlignment(Qt::AlignHCenter);
    m_activateStatus->setText(tr("Unactivated"));
    m_activateStatus->hide();

    //创建标题栏右侧菜单按钮
    auto btnForMenu = new QPushButton(this);
    btnForMenu->setObjectName("btnForMenu");
    btnForMenu->setFixedSize(QSize(16, 16));

    auto settingMenu = new QMenu(this);
    btnForMenu->setMenu(settingMenu);
    settingMenu->setObjectName("settingMenu");

    settingMenu->addAction(tr("Settings"), this, &Window::popupSettingsDialog);
    settingMenu->addAction(tr("Activation"), this, &Window::popupActiveDialog);
    settingMenu->addAction(tr("About"), this, &Window::popupAboutDialog);

    layout->addWidget(m_activateStatus);
    layout->addWidget(btnForMenu);
    layout->setAlignment(Qt::AlignRight);

    connect(m_ui->m_sidebar, &SideBar::itemChanged, this, &Window::updateSidebar);
}

void Window::initNavigation()
{
    // 初始化分类选项
    m_ui->m_navigation->addItem(new NavigationItem(":/images/baseline-reinforcement", tr("Baseline reinforcement")));
    m_ui->m_navigation->addItem(new NavigationItem(":/images/trusted-protected", tr("Trusted protected")));
    m_ui->m_navigation->addItem(new NavigationItem(":/images/file-protected", tr("File protected")));
    m_ui->m_navigation->addItem(new NavigationItem(":/images/box-manager", tr("Private box")));
    m_ui->m_navigation->addItem(new NavigationItem(":/images/device", tr("Device management")));
    m_ui->m_navigation->setBtnChecked(0);

    // 移除qt designer默认创建的widget
    while (m_ui->m_stackedPages->currentWidget() != nullptr)
    {
        auto currentWidget = m_ui->m_stackedPages->currentWidget();
        m_ui->m_stackedPages->removeWidget(currentWidget);
        delete currentWidget;
    }

    addPage(new BR::BRPage(this));
    // 可信保护页面需判断是否加载成功
    auto execute = new TP::ExecuteProtectedPage(this);
    addPage(execute);
    addPage(new TP::KernelProtectedPage(this));
    addPage(new FP::FileProtectionPage(this));
    addPage(new PrivateBox::BoxPage(this));
    addPage(new DM::DeviceListPage(this));
    addPage(new DM::DeviceLogPage(this));
    // 页面加载动画
    m_loading = new Loading(this);
    m_loading->setFixedSize(execute->size());
    m_ui->m_stackedPages->addWidget(m_loading);

    m_ui->m_stackedPages->setCurrentIndex(0);
    updatePage();

    connect(execute, &TP::ExecuteProtectedPage::initFinished, this, [this] {
        m_loading->setVisible(false);
        m_ui->m_sidebar->setEnabled(true);
        updatePage();
    });

    connect(m_ui->m_navigation, SIGNAL(currentUIDChanged()), this, SLOT(updatePage()));
}

void Window::initSettings()
{
    Settings::Dialog::globalInit(this);
    // 导出策略需要从表格中获取勾选项，设置页面中无法获取，通过信号实现
    disconnect(Settings::Dialog::instance(), &Settings::Dialog::exportStrategyClicked, nullptr, nullptr);
    connect(Settings::Dialog::instance(), &Settings::Dialog::exportStrategyClicked, this, [this] {
        for (auto page : m_pages.value(tr("Baseline reinforcement")))
        {
            if (page->isVisible())
            {
                auto brPage = static_cast<BR::BRPage *>(page);
                brPage->exportStrategy();
            }
        }
    });
    disconnect(Settings::Dialog::instance(), &Settings::Dialog::exportStrategyClicked, nullptr, nullptr);
    connect(Settings::Dialog::instance(), &Settings::Dialog::resetAllArgsClicked, this, [this] {
        for (auto page : m_pages.value(tr("Baseline reinforcement")))
        {
            if (page->isVisible())
            {
                auto brPage = static_cast<BR::BRPage *>(page);
                brPage->resetAllReinforcementArgs();
            }
        }
    });
}

void Window::addPage(Page *page)
{
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
    //设置激活对话框和激活状态标签是否可见
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
}  // namespace KS
