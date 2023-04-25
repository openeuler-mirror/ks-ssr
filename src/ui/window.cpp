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
#include <QMutex>
#include <QPushButton>
#include <QStackedWidget>
#include "src/ui/box/box-page.h"
#include "src/ui/device/device-page.h"
#include "src/ui/file-protected/fp-page.h"
#include "src/ui/navigation.h"
#include "src/ui/trusted/tp-page.h"
#include "src/ui/ui_window.h"

namespace KS
{
#define KSC_STYLE_PATH ":/styles/ksc"

Window::Window() : TitlebarWindow(nullptr), m_ui(new Ui::Window)
{
    m_ui->setupUi(getWindowContentWidget());

    initWindow();
    initNavigation();
}

Window::~Window()
{
    delete m_ui;
}

void Window::initWindow()
{
    setTitle(tr("Security control"));
    setIcon(QIcon(":/images/logo"));
    setFixedWidth(984);
    setFixedHeight(648);

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
}

void Window::initNavigation()
{
    // 初始化分类选项
    m_ui->m_navigation->addItem(new NavigationItem(":/images/trusted-protected", tr("Trusted protected")));
    m_ui->m_navigation->addItem(new NavigationItem(":/images/file-protected", tr("File protected")));
    m_ui->m_navigation->addItem(new NavigationItem(":/images/box-manager", tr("Private box")));
    m_ui->m_navigation->addItem(new NavigationItem(":/images/box-manager", tr("Peripheral management")));
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
}  // namespace KS
