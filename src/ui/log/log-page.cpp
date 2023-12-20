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

#include "src/ui/log/log-page.h"
#include <qt5-log-i.h>
#include <QWidgetAction>
#include "config.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/log_proxy.h"
#include "src/ui/ui_log-page.h"
#include "ssr-i.h"

namespace KS
{
namespace Log
{
LogPage::LogPage(QWidget *parent)
    : Page(parent),
      m_ui(new Ui::LogPage())
{
    m_ui->setupUi(this);

    m_logProxy = new LogProxy(SSR_DBUS_NAME,
                              "/com/kylinsec/SSR/Log",
                              QDBusConnection::systemBus(),
                              this);
    // TODO 更新表格右上角提示信息
    //    auto text = QString(tr("A total of %1 records")).arg(m_ui->m_logProxy->getLogInfos().size());
    //    m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setObjectName("searchButton");
    searchButton->setIcon(QIcon(":/images/search"));
    searchButton->setIconSize(QSize(16, 16));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);
    // TODO 发送key给后台
    //    connect(m_ui->m_search, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged(const QString &)));
    connect(m_ui->m_logTable, &LogTable::logUpdated, this, &LogPage::updateTips);
}

LogPage::~LogPage()
{
    delete m_ui;
}

QString LogPage::getNavigationUID()
{
    return tr("Log auditd");
}

QString LogPage::getSidebarUID()
{
    return "";
}

QString LogPage::getSidebarIcon()
{
    return "";
}

QString LogPage::getAccountRoleName()
{
    return SSR_ACCOUNT_NAME_AUDADM;
}

void LogPage::searchTextChanged(const QString &text)
{
    m_ui->m_logTable->searchTextChanged(text);
}

void LogPage::updateTips(int total)
{
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(QString::number(total));
    m_ui->m_tips->setText(text);
}
}  // namespace Log
}  // namespace KS
