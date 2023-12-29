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

#include "include/ssr-i.h"
#include "src/ui/common/page.h"

class QStackedWidget;

namespace KS
{
namespace BR
{
class Home;
class Scan;

enum BRPageType
{
    BR_PAGE_TYPE_HOME = 0,
    BR_PAGE_TYPE_SCAN,
    BR_PAGE_TYPE_OTHER
};

class BRPage : public Page
{
    Q_OBJECT
public:
    BRPage(QWidget *parent = nullptr);
    virtual ~BRPage(){};

    QString getNavigationUID() override;
    QString getSidebarUID() override;
    QString getSidebarIcon() override;
    QString getAccountRoleName() override;
    // 导出策略需要从表格中获取勾选项，设置页面中无法获取，通过信号实现
    bool exportStrategy();
    void resetAllReinforcementArgs();

private:
    void initUI();
    void replaceWidget(BRPageType type);

private slots:
    void startSystemScan();
    void startCustomScan();
    void returnHome();

private:
    QStackedWidget *m_stacked;
    Home *m_home;
    Scan *m_scan;
    BRStrategyType m_strategyType;
};

}  // namespace BR
}  // namespace KS
