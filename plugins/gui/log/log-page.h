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

#pragma once

#include <page.h>
#include <QWidget>

class LogProxy;

namespace Ui
{
class LogPage;
}  // namespace Ui

namespace KS
{
class Pagination;
class DatePicker;

namespace Log
{
class LogPage : public WorkPage
{
    Q_OBJECT

public:
    LogPage(QWidget *parent = nullptr);
    virtual ~LogPage();

    NavigationIndex getNavigationIndex() override;
    QString getSidebarUID() override;
    QString getSidebarIcon() override;
    QString getAccountRoleName() override;
    bool isTaskRunning() override;
    bool stopTask() override;

private:
    void initUI();

private Q_SLOTS:
    void updateTipsAndPagination(int total);

private:
    Ui::LogPage *m_ui;
    Pagination *m_pagination;
    LogProxy *m_logProxy;
    DatePicker *m_datePicker;
};
}  // namespace Log
}  // namespace KS
