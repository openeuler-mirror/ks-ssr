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
#include "progress.h"

namespace Ui
{
class BRSettingPage;
}

class BRDbusProxy;

namespace KS
{
namespace BR
{
class Category;

class BRSettingPage : public SettingPage
{
    Q_OBJECT

public:
    explicit BRSettingPage(QWidget *parent = nullptr);
    virtual ~BRSettingPage();
    uint getFallbackStatus();

    virtual QString getTitle();

private:
    void initConnection();
    void initUI();
    void updateProgressInfo(KS::BR::ProgressInfo &progressInfo);

private slots:
    void importStrategy();
    void timedScanSettings(int hours);
    void scan();
    void setMonitorStatus(bool isOpen);
    void fallback(int status);

signals:
    // 需要通过勾选项进行导出，设置界面无法获取到表格勾选信息，需要在page中获取
    void exportStrategyClicked();
    // 重置所有加固项需要与表格界面交互
    void resetAllArgsClicked();

private:
    Ui::BRSettingPage *m_ui;

    QTimer *m_timedScan;
    QList<KS::BR::Category *> m_categories = {};
    KS::BR::ProgressInfo m_progressInfo = {};

    BRDbusProxy *m_dbusProxy;
};
}  // namespace BR
}  // namespace KS
