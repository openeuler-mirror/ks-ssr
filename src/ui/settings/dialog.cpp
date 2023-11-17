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
#include "dialog.h"
#include <QIcon>
#include "src/ui/settings/device-control.h"
#include "src/ui/settings/trusted-protected.h"
#include "src/ui/settings/baseline-reinforcement.h"
#include "ui_dialog.h"

namespace KS
{
namespace Settings
{
Dialog *Dialog::m_instance = nullptr;

void Dialog::globalInit(QWidget *parent)
{
    m_instance = new Dialog(parent);
};
void Dialog::globalDeinit()
{
    if (m_instance)
    {
        delete m_instance;
        m_instance = nullptr;
    }
};

Dialog::Dialog(QWidget *parent) : TitlebarWindow(parent),
                                  m_ui(new Ui::Dialog)
{
    m_ui->setupUi(getWindowContentWidget());
    initUI();
    initSidebar();
    initSubPage();
    hide();

    connect(m_ui->m_sidebar, &QListWidget::currentRowChanged, m_ui->m_stacked, &QStackedWidget::setCurrentIndex);
}

Dialog::~Dialog()
{
    delete m_ui;
}

void Dialog::closeEvent(QCloseEvent *event)
{
    hide();
    Q_ASSERT(event);
    return;
}

void Dialog::initUI()
{
    setTitle(tr("Settings"));
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
}

void Dialog::initSidebar()
{
    auto reinforceItem = new QListWidgetItem(m_ui->m_sidebar);
    reinforceItem->setText(tr("Baseline reinforcement"));
    reinforceItem->setTextAlignment(Qt::AlignCenter);
    reinforceItem->setSizeHint(QSize(110, 42));

    auto trustedItem = new QListWidgetItem(m_ui->m_sidebar);
    trustedItem->setText(tr("Trusted protect"));
    trustedItem->setTextAlignment(Qt::AlignCenter);
    trustedItem->setSizeHint(QSize(110, 42));

    auto deviceItem = new QListWidgetItem(m_ui->m_sidebar);
    deviceItem->setText(tr("Interface Control"));
    deviceItem->setTextAlignment(Qt::AlignCenter);
    deviceItem->setSizeHint(QSize(110, 42));

    m_ui->m_sidebar->addItem(reinforceItem);
    m_ui->m_sidebar->addItem(trustedItem);
    m_ui->m_sidebar->addItem(deviceItem);
    m_ui->m_sidebar->setCurrentRow(0);
}

void Dialog::initSubPage()
{
    auto reinforceSettings = new BaselineReinforcement(this);
    auto trustedSettings = new TrustedProtected(this);
    auto deviceSettings = new DeviceControl(this);

    connect(reinforceSettings, &BaselineReinforcement::exportStrategyClicked, this, &Dialog::exportStrategyClicked);
    connect(reinforceSettings, &BaselineReinforcement::resetAllArgsClicked, this, &Dialog::resetAllArgsClicked);

    m_ui->m_stacked->addWidget(reinforceSettings);
    m_ui->m_stacked->addWidget(trustedSettings);
    m_ui->m_stacked->addWidget(deviceSettings);
    m_ui->m_stacked->setCurrentIndex(0);
}
}  // namespace Settings
}  // namespace KS
