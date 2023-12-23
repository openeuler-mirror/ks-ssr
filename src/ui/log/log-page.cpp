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
#include <QDateTime>
#include <QWidgetAction>
#include "config.h"
#include "src/ui/common/date-picker/date-picker.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/common/table/pagination.h"
#include "src/ui/log_proxy.h"
#include "src/ui/ui_log-page.h"
#include "ssr-i.h"

namespace KS
{
namespace Log
{
#define MAX_PAGINATION_BUTTON_NUMBER 6

LogPage::LogPage(QWidget *parent)
    : Page(parent),
      m_ui(new Ui::LogPage())
{
    m_ui->setupUi(this);

    m_logProxy = new LogProxy(SSR_DBUS_NAME,
                              "/com/kylinsec/SSR/Log",
                              QDBusConnection::systemBus(),
                              this);
    initUI();
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

void LogPage::initUI()
{
    auto text = QString(tr("A total of %1 records")).arg(m_ui->m_logTable->getLogNumbers());
    m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setObjectName("searchButton");
    searchButton->setIcon(QIcon(":/images/search"));
    searchButton->setIconSize(QSize(16, 16));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);
    connect(m_ui->m_centigrade, &QPushButton::clicked, this, [this] {
        m_ui->m_logTable->search(m_ui->m_search->text());
    });
    connect(m_ui->m_logTable, &LogTable::logUpdated, this, &LogPage::updateTipsAndPagination);

    // 分页
    m_pagination = new Pagination(m_ui->m_logTable->getLogNumbers() / LOG_PAGE_NUMBER,
                                  MAX_PAGINATION_BUTTON_NUMBER,
                                  true,
                                  this);
    connect(m_pagination, &Pagination::currentPageChanged, this, [this](int number) {
        m_ui->m_logTable->setCurrentPage(uint(number));
    });
    m_ui->m_mainLayout->addWidget(m_pagination);

    // 日期选择
    // 当前日期的前一个月到现在
    m_ui->m_calendarButton->setStartDate(QDateTime::currentDateTime().addMonths(-1).toString("yyyy-MM-dd"));
    m_ui->m_calendarButton->setEndDate(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    m_datePicker = new DatePicker(this);
    connect(m_datePicker, &DatePicker::startDateChanged, this, [this](const QString &date) {
        m_ui->m_calendarButton->setStartDate(date);
        auto beginTime = QDateTime::fromString(date, "yyyy-MM-dd");
        beginTime.setTime(QTime(0, 0, 0));
        m_ui->m_logTable->setTimeStampBegin(beginTime.toSecsSinceEpoch());
    });
    connect(m_datePicker, &DatePicker::endDateChanged, this, [this](const QString &date) {
        m_ui->m_calendarButton->setEndDate(date);
        auto endTime = QDateTime::fromString(date, "yyyy-MM-dd");
        endTime.setTime(QTime(23, 59, 59));
        m_ui->m_logTable->setTimeStampBegin(endTime.toSecsSinceEpoch());
    });

    connect(m_ui->m_calendarButton, &DatePickButton::fristDateClicked, this, [this] {
        m_datePicker->move(QCursor::pos());
        m_datePicker->showDatePicker(0);
        m_datePicker->show();
    });
    connect(m_ui->m_calendarButton, &DatePickButton::endDateClicked, this, [this] {
        m_datePicker->move(QCursor::pos());
        m_datePicker->showDatePicker(1);
        m_datePicker->show();
    });
}

void LogPage::updateTipsAndPagination(int total)
{
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(QString::number(total));
    m_ui->m_tips->setText(text);
    m_pagination->setTotalPage(total / LOG_PAGE_NUMBER);
}
}  // namespace Log
}  // namespace KS
