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
#include "br-page.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include "include/ssr-i.h"
#include "src/ui/br/home.h"
#include "src/ui/br/scan.h"
#include "src/ui/common/ssr-marcos-ui.h"

namespace KS
{
namespace BR
{
BRPage::BRPage(QWidget *parent)
    : Page(parent),
      m_strategyType(BR_STRATEGY_TYPE_SYSTEM)
{
    initUI();
}

QString BRPage::getNavigationUID()
{
    return tr("Baseline reinforcement");
}

QString BRPage::getSidebarUID()
{
    return "";
}

QString BRPage::getSidebarIcon()
{
    return "";
}

QString BRPage::getAccountRoleName()
{
    return SSR_ACCOUNT_NAME_SYSADM;
}

bool BRPage::exportStrategy()
{
    return m_scan->exportStrategy();
}

void BRPage::resetAllReinforcementArgs()
{
    m_scan->resetAllReinforcementItem();
    POPUP_MESSAGE_DIALOG(tr("Reset success!"));
}

void BRPage::initUI()
{
    auto layout = new QVBoxLayout(this);
    m_stacked = new QStackedWidget(this);
    m_home = new Home(m_stacked);
    m_scan = new Scan(m_stacked);
    m_stacked->addWidget(m_home);
    m_stacked->addWidget(m_scan);
    layout->addWidget(m_stacked);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(m_home, &Home::systemScanClicked, this, &BRPage::startSystemScan);
    connect(m_home, &Home::customScanClicked, this, &BRPage::startCustomScan);
    connect(m_home, &Home::currentStrategyChanged, this, [this](int type)
            {
                m_strategyType = BRStrategyType(type);
            });
    connect(m_scan, &Scan::returnHomeClicked, this, &BRPage::returnHome);
    connect(m_scan, &Scan::reinforcementFinished, m_home, &Home::modfiyReinforcementTime);
}

void BRPage::replaceWidget(BRPageType type)
{
    m_stacked->setCurrentIndex(type);
}

void BRPage::startSystemScan()
{
    replaceWidget(BR_PAGE_TYPE_SCAN);
    m_scan->usingSystemStrategy();
    m_scan->emitScanSignal();
}

void BRPage::startCustomScan()
{
    replaceWidget(BR_PAGE_TYPE_SCAN);
    // TODO 切换页面后考虑是否还有其它操作
    m_scan->usingCustomStrategy();
}

void BRPage::returnHome()
{
    replaceWidget(BR_PAGE_TYPE_HOME);
    m_scan->reset();
}
}  // namespace BR
}  // namespace KS
