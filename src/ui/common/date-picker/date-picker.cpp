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
#include "date-picker.h"
#include <QDateTime>
#include <QHBoxLayout>
#include <QStackedWidget>
#include "calendar-widget.h"

namespace KS
{
DatePicker::DatePicker(QWidget *parent)
    : QWidget(parent), m_startCalendar(nullptr), m_endCalendar(nullptr), m_stackedWidget(nullptr)
{
    initUI();
}

DatePicker::~DatePicker()
{
}

QDateTime DatePicker::getStartDate()
{
    QDate startDate = m_startCalendar->getSelectDate();
    QDate endDate = m_endCalendar->getSelectDate();
    QDateTime start = QDateTime(startDate);
    if (startDate == endDate)
    {
        start.setTime(QTime(0, 0, 0));
    }
    return start;
}

QDateTime DatePicker::getEndDate()
{
    QDate startDate = m_startCalendar->getSelectDate();
    QDate endDate = m_endCalendar->getSelectDate();
    QDateTime end = QDateTime(endDate);
    if (startDate == endDate)
    {
        end.setTime(QTime(23, 59, 59));
    }
    return end;
}

void DatePicker::showDatePicker(int type)
{
    m_stackedWidget->setCurrentIndex(type);
}

void DatePicker::changeStartDate(QDate date)
{
    m_startDate = date;
    m_endCalendar->setMinimumDate(date);
    m_endCalendar->setSelectableStart(date);
    m_endCalendar->setSelectableEnd(m_endCalendar->selectedDate());
    m_startCalendar->setSelectableStart(date);
    m_startCalendar->setSelectableEnd(m_endCalendar->selectedDate());
    setDateLimit();
    emit startDateChanged(date.toString("yyyy-MM-dd"));
}

void DatePicker::changeEndDate(QDate date)
{
    m_endDate = date;
    m_endCalendar->setSelectableStart(m_startCalendar->selectedDate());
    m_endCalendar->setSelectableEnd(date);
    m_startCalendar->setSelectableStart(m_startCalendar->selectedDate());
    m_startCalendar->setSelectableEnd(date);
    setDateLimit();
    emit endDateChanged(date.toString("yyyy-MM-dd"));
}

void DatePicker::initUI()
{
    setWindowFlags(Qt::Widget | Qt::Popup | Qt::FramelessWindowHint);
    setMinimumSize(200, 300);
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(10);
    setLayout(mainLayout);

    m_stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(m_stackedWidget);

    m_endDate = QDate::currentDate();
    m_startDate = m_endDate.addMonths(-1);

    m_startCalendar = new CalendarWidget(m_stackedWidget);
    // TODO 确认是否有需求现在最大和最小可选数
    m_startCalendar->setSelectedDate(m_startDate);
    m_startCalendar->setSelectableStart(QDate::currentDate().addMonths(-1));
    m_startCalendar->setMinimumDate(QDate::currentDate().addMonths(-1));
    connect(m_startCalendar, &CalendarWidget::clicked, this, &DatePicker::changeStartDate);

    m_endCalendar = new CalendarWidget(m_stackedWidget);
    //     m_endCalendar->hidePreButton();
    m_endCalendar->setSelectableEnd(QDate::currentDate());
    m_endCalendar->setMaximumDate(QDate::currentDate());
    m_endCalendar->setSelectedDate(m_endDate);
    connect(m_endCalendar, &CalendarWidget::clicked, this, &DatePicker::changeEndDate);

    setDateLimit();

    m_stackedWidget->addWidget(m_startCalendar);
    m_stackedWidget->addWidget(m_endCalendar);
}

void DatePicker::setDateLimit()
{
    m_startCalendar->setSelectableEnd(m_endDate.addDays(-1));
    m_startCalendar->setMaximumDate(m_endDate.addDays(-1));

    m_endCalendar->setSelectableStart(m_startDate.addDays(1));
    m_endCalendar->setMinimumDate(m_startDate.addDays(1));
}
}  // namespace KS
