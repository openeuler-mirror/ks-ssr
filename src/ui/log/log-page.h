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

#include "src/ui/common/page.h"

class LogProxy;

namespace Ui
{
class LogPage;
}  // namespace Ui

namespace KS
{
namespace Log
{
class LogPage : public Page
{
    Q_OBJECT
public:
    LogPage(QWidget *parent = nullptr);
    virtual ~LogPage();

    QString getNavigationUID() override;
    QString getSidebarUID() override;
    QString getSidebarIcon() override;
    int getSelinuxType() override;

private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void updateTips(int total);

private:
    Ui::LogPage *m_ui;

    LogProxy *m_logProxy;
};
}  // namespace Log
}  // namespace KS
