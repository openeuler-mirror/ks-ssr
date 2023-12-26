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
#include "include/ssr-marcos.h"
#include "src/ui/settings/baseline-reinforcement.h"
#include "src/ui/settings/device-control.h"
#include "src/ui/settings/trusted-protected.h"
#include "src/ui/settings/identity-authentication.h"
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
}

void Dialog::addSidebars(const QStringList &sidebarNames)
{
    for (auto sidebarName : sidebarNames)
    {
        CONTINUE_IF_TRUE(sidebarName.isEmpty());

        auto item = new QListWidgetItem(m_ui->m_sidebar);
        item->setText(sidebarName);
        item->setTextAlignment(Qt::AlignCenter);
        item->setSizeHint(QSize(110, 42));
        m_ui->m_sidebar->addItem(item);

        addSubPage(sidebarName);
    }
    m_ui->m_sidebar->setCurrentRow(0);
    m_ui->m_stacked->setCurrentIndex(0);
};

Dialog::Dialog(QWidget *parent)
    : TitlebarWindow(parent),
      m_ui(new Ui::Dialog)
{
    m_ui->setupUi(getWindowContentWidget());
    initUI();
    hide();

    connect(m_ui->m_sidebar, &QListWidget::currentRowChanged, m_ui->m_stacked, &QStackedWidget::setCurrentIndex);
}

Dialog::~Dialog()
{
    delete m_ui;
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

void Dialog::addSubPage(const QString &sidebarName)
{
    if (sidebarName == tr("Baseline reinforcement"))
    {
        auto reinforceSettings = new BaselineReinforcement(this);
        connect(reinforceSettings, &BaselineReinforcement::exportStrategyClicked, this, &Dialog::exportStrategyClicked);
        connect(reinforceSettings, &BaselineReinforcement::resetAllArgsClicked, this, &Dialog::resetAllArgsClicked);
        m_ui->m_stacked->addWidget(reinforceSettings);
    }
    else if (sidebarName == tr("Trusted protect"))
    {
        auto trustedSettings = new TrustedProtected(this);
        m_ui->m_stacked->addWidget(trustedSettings);
    }
    else if (sidebarName == tr("Interface Control"))
    {
        auto deviceSettings = new DeviceControl(this);
        m_ui->m_stacked->addWidget(deviceSettings);
    }
    else if (sidebarName == tr("Identity authentication"))
    {
        auto identityAuthentication = new IdentityAuthentication(this);
        m_ui->m_stacked->addWidget(identityAuthentication);
    }
}
}  // namespace Settings
}  // namespace KS
