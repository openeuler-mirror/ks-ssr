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
#include "settings.h"
#include <ssr-marcos.h>
#include <QIcon>
#include "ui_settings.h"

namespace KS
{
Settings::Settings(QWidget *parent)
    : TitlebarWindow(parent),
      m_ui(new Ui::Settings)
{
    m_ui->setupUi(getWindowContentWidget());
    initUI();
    hide();

    connect(m_ui->m_sidebar, &QListWidget::currentRowChanged, m_ui->m_stacked, &QStackedWidget::setCurrentIndex);
}

Settings::~Settings()
{
    delete m_ui;
}

void Settings::setSettingPages(const QVector<SettingPage *> &settingPages)
{
    for (auto settingPage : settingPages)
    {
        auto title = settingPage->getTitle();
        if (title.isEmpty())
        {
            KLOG_WARNING() << "The title of setting page is empty, so ignore it.";
            continue;
        }

        auto item = new QListWidgetItem(m_ui->m_sidebar);
        item->setText(title);
        item->setTextAlignment(Qt::AlignCenter);
        item->setSizeHint(QSize(110, 42));
        m_ui->m_sidebar->addItem(item);
        m_ui->m_stacked->addWidget(settingPage);
    }

    if (m_ui->m_sidebar->count() > 0 && m_ui->m_stacked->count() > 0)
    {
        m_ui->m_sidebar->setCurrentRow(0);
        m_ui->m_stacked->setCurrentIndex(0);
    }
    else
    {
        KLOG_INFO() << "No setting pages";
    }

    // TODO: 放到插件实现
    // connect(reinforceSettings, &BaselineReinforcement::exportStrategyClicked, this, &Settings::exportStrategyClicked);
    // connect(reinforceSettings, &BaselineReinforcement::resetAllArgsClicked, this, &Settings::resetAllArgsClicked);
}

void Settings::clearSettingPages()
{
    m_ui->m_sidebar->clear();

    while (m_ui->m_stacked->currentWidget() != nullptr)
    {
        auto currentWidget = m_ui->m_stacked->currentWidget();
        m_ui->m_stacked->removeWidget(currentWidget);
        delete currentWidget;
    }
}

void Settings::initUI()
{
    setTitle(tr("Settings"));
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setResizeable(false);
    setTitleBarHeight(36);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
}

}  // namespace KS
