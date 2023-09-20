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
#include "settings-page.h"
#include <QIcon>
#include "src/ui/setup/trusted-settings.h"
#include "ui_settings-page.h"

namespace KS
{
SettingsPage::SettingsPage(QWidget *parent) : TitlebarWindow(parent),
                                              m_ui(new Ui::SettingsPage)
{
    m_ui->setupUi(getWindowContentWidget());
    initUI();
    initSidebar();
    initSubPage();

    connect(m_ui->m_sidebar, &QListWidget::currentRowChanged, m_ui->m_stacked, &QStackedWidget::setCurrentIndex);
}

SettingsPage::~SettingsPage()
{
    delete m_ui;
}

void SettingsPage::initUI()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setTitle(tr("Settings"));
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
}

void SettingsPage::initSidebar()
{
    auto trustedItem = new QListWidgetItem(m_ui->m_sidebar);
    trustedItem->setText(tr("Trusted protect"));
    trustedItem->setTextAlignment(Qt::AlignCenter);
    trustedItem->setSizeHint(QSize(110, 42));

    m_ui->m_sidebar->addItem(trustedItem);
    m_ui->m_sidebar->setCurrentRow(0);
}

void SettingsPage::initSubPage()
{
    auto trustedSettings = new TrustedSettings(this);
    connect(trustedSettings, &TrustedSettings::trustedStatusChange, this, &SettingsPage::trustedStatusChange);

    m_ui->m_stacked->addWidget(trustedSettings);
    m_ui->m_stacked->setCurrentIndex(0);
}
}  // namespace KS
