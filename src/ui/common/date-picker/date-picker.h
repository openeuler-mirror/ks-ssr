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

#pragma once

#include <QDate>
#include <QWidget>

class QStackedWidget;
class QDateTime;

namespace KS
{
class CalendarWidget;
class DatePicker : public QWidget
{
    Q_OBJECT
public:
    explicit DatePicker(QWidget *parent = nullptr);
    ~DatePicker();
    // start:0  end:1
    void showDatePicker(int type);

private slots:
    void changeStartDate(QDate date);
    void changeEndDate(QDate date);

signals:
    void startDateChanged(const QString &);
    void endDateChanged(const QString &);

private:
    void initUI();
    void setDateLimit();

private:
    CalendarWidget *m_startCalendar;
    CalendarWidget *m_endCalendar;
    QDate m_startDate;
    QDate m_endDate;
    QStackedWidget *m_stackedWidget;
};
}  // namespace KS
