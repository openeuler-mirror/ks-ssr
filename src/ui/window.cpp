/**
 * @Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
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

#include "src/ui/window.h"
#include <qt5-log-i.h>
#include <QFile>
#include <QMenu>
#include <QMutex>
#include <QPushButton>
#include <QStackedWidget>
#include "src/ui/box/box-page.h"
#include "src/ui/device/device-page.h"
#include "src/ui/fp/fp-page.h"
#include "src/ui/license/license-activation.h"
#include "src/ui/license/license-dbus.h"
#include "src/ui/navigation.h"
#include "src/ui/tp/tp-page.h"
#include "src/ui/ui_window.h"

namespace KS
{
#define KSC_STYLE_PATH ":/styles/ksc"

Window::Window() : TitlebarWindow(nullptr),
                   m_ui(new Ui::Window),
                   m_activation(nullptr),
                   m_activateStatus(nullptr)
{
    m_ui->setupUi(getWindowContentWidget());

    initWindow();
    initNavigation();
    initActivation();
}

Window::~Window()
{
    delete m_ui;
}

void Window::initActivation()
{
    m_activation = new LicenseActivation(this);
    if (!m_activation->isActivate())
    {
        m_activateStatus->show();
        auto x = this->x() + this->width() / 4 + m_activation->width() / 4;
        auto y = this->y() + this->height() / 4 + m_activation->height() / 4;
        m_activation->move(x, y);
        m_activation->raise();
        m_activation->show();
    }
    connect(m_activation, &LicenseActivation::activated,
            [this](bool isActived)
            {
                //设置激活对话框和激活状态标签是否可见
                m_activation->setVisible(!isActived);
                m_activateStatus->setVisible(!isActived);
            });
    connect(m_activation, &LicenseActivation::closed,
            [this]
            {
                //未激活状态下获取关闭信号，则退出程序;已激活状态下后获取关闭信号，只是隐藏激活对话框
                if (!m_activation->isActivate())
                    qApp->quit();
                else
                    m_activation->hide();
            });
}

void Window::initWindow()
{
    setTitle(tr("Security control"));
    setIcon(QIcon(":/images/logo"));
    setFixedSize(984, 648);

    // 初始化样式表
    QFile file(KSC_STYLE_PATH);
    if (file.open(QIODevice::ReadOnly))
    {
        QString windowStyle = file.readAll();
        setStyleSheet(styleSheet() + windowStyle);
    }
    else
    {
        KLOG_WARNING() << "Failed to open file " << KSC_STYLE_PATH;
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
    m_activateStatus->setText("Unactivated");
    m_activateStatus->hide();

    //创建标题栏右侧菜单按钮
    auto btnForMenu = new QPushButton(this);
    btnForMenu->setObjectName("btnForMenu");
    btnForMenu->setFixedSize(QSize(16, 16));

    auto settingMenu = new QMenu(this);
    auto action = settingMenu->addAction(tr("Activate Info"));
    connect(action, &QAction::triggered, this, &Window::popupActiveDialog);
    btnForMenu->setMenu(settingMenu);

    layout->addWidget(m_activateStatus);
    layout->addWidget(btnForMenu);
    layout->setAlignment(Qt::AlignRight);
}

void Window::initNavigation()
{
    // 初始化分类选项
    m_ui->m_navigation->addItem(new NavigationItem(":/images/trusted-protected", tr("Trusted protected")));
    m_ui->m_navigation->addItem(new NavigationItem(":/images/file-protected", tr("File protected")));
    m_ui->m_navigation->addItem(new NavigationItem(":/images/box-manager", tr("Private box")));
    m_ui->m_navigation->addItem(new NavigationItem(":/images/device", tr("Device management")));
    m_ui->m_navigation->setBtnChecked(0);

    // 移除qt designer默认创建的widget
    while (m_ui->m_pages->currentWidget() != nullptr)
    {
        auto currentWidget = m_ui->m_pages->currentWidget();
        m_ui->m_pages->removeWidget(currentWidget);
        delete currentWidget;
    }

    m_ui->m_pages->addWidget(new TPPage(this));
    m_ui->m_pages->addWidget(new FPPage());
    m_ui->m_pages->addWidget(new BoxPage());
    m_ui->m_pages->addWidget(new DevicePage());
    m_ui->m_pages->setCurrentIndex(0);

    connect(m_ui->m_navigation, SIGNAL(currentCategoryChanged(int)), m_ui->m_pages, SLOT(setCurrentIndex(int)));
}

void Window::popupActiveDialog()
{
    m_activation->update();
    auto x = this->x() + this->width() / 4 + m_activation->width() / 4;
    auto y = this->y() + this->height() / 4 + m_activation->height() / 4;
    m_activation->move(x, y);
    m_activation->show();
}
}  // namespace KS
