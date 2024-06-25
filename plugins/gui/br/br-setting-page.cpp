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

#include "br-setting-page.h"
#include <qt5-log-i.h>
#include <ssr-i.h>
#include "br-i.h"
#include "br_dbus_proxy.h"
#include "lib/base/notification-wrapper.h"
#include "lib/widgets/ssr-marcos-ui.h"
#include "lib/widgets/user-prompt-dialog.h"
#include "reinforcement-items/category.h"
#include "ui_br-setting-page.h"
#include "utils.h"

#define STYLE_PATH ":/br/style/setting-page.qss"

namespace KS
{
namespace BR
{
BRSettingPage::BRSettingPage(QWidget *parent)
    : SettingPage(parent),
      m_ui(new Ui::BRSettingPage)
{
    m_ui->setupUi(this);

    m_dbusProxy = new BRDbusProxy(SSR_DBUS_NAME,
                                  BR_DBUS_OBJECT_PATH,
                                  QDBusConnection::systemBus(),
                                  this);
    initConnection();
    initUI();
}

BRSettingPage::~BRSettingPage()
{
    delete m_ui;
}

void BRSettingPage::initConnection()
{
    connect(m_ui->m_timeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &BRSettingPage::timedScanSettings);
    m_timedScan = new QTimer(this);
    connect(m_timedScan, &QTimer::timeout, this, &BRSettingPage::scan);
    connect(m_ui->m_openMonitor, &QRadioButton::clicked, [this]
            {
                m_dbusProxy->SetResourceMonitorSwitch(BR_RESOURCE_MONITOR_OPEN);
                if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
                {
                    Notify::NOTIFY_INFO(tr("Resource monitor open!").toUtf8());
                }
                KLOG_INFO() << "Resource monitor open!";
            });
    connect(m_ui->m_closeMonitor, &QRadioButton::clicked, [this]
            {
                m_dbusProxy->SetResourceMonitorSwitch(BR_RESOURCE_MONITOR_CLOSE);
                if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
                {
                    Notify::NOTIFY_INFO(tr("Resource monitor close!").toUtf8());
                }
                KLOG_INFO() << "Resource monitor close!";
            });

    connect(m_ui->m_openNotify, &QRadioButton::clicked, [this]
            {
                m_dbusProxy->SetNotificationStatus(BR_NOTIFICATION_STATUS_OPEN);
                Notify::NOTIFY_INFO(tr("Notify open!").toUtf8());
                KLOG_INFO() << "Notify open!";
            });
    connect(m_ui->m_closeNotify, &QRadioButton::clicked, [this]
            {
                m_dbusProxy->SetNotificationStatus(BR_NOTIFICATION_STATUS_CLOSE);
                Notify::NOTIFY_INFO(tr("Notify close!").toUtf8());
                KLOG_INFO() << "Notify close!";
            });

    connect(m_ui->m_fallbackInit, &QPushButton::clicked, this, [this]
            {
                if (m_dbusProxy->GetDispatchStatus() != BRDispatchState::BR_DISPATCH_STATE_IDLE)
                {
                    POPUP_MESSAGE_DIALOG(tr("A job is in progress, please wait."));
                    return;
                }
                auto userPrompt = new UserPromptDialog(parentWidget());
                userPrompt->setNotifyMessage(tr("Fallback"), tr("Are you sure you want to go back to the initialization state?"));
                auto x = window()->x() + window()->width() / 2 - userPrompt->width() / 2;
                auto y = window()->y() + window()->height() / 2 - userPrompt->height() / 2;
                userPrompt->move(x, y);
                userPrompt->show();
                connect(userPrompt, &UserPromptDialog::accepted, this, [this]
                        {
                            fallback(BRFallbackMethod::BR_FALLBACK_METHOD_INITIAL);
                        });
            });
    connect(m_ui->m_fallbackPrevious, &QPushButton::clicked, this, [this]
            {
                if (m_dbusProxy->GetDispatchStatus() != BRDispatchState::BR_DISPATCH_STATE_IDLE)
                {
                    POPUP_MESSAGE_DIALOG(tr("A job is in progress, please wait."));
                    return;
                }
                auto userPrompt = new UserPromptDialog(parentWidget());
                userPrompt->setNotifyMessage(tr("Fallback"), tr("Are you sure you want to go back to the previous state?"));
                auto x = window()->x() + window()->width() / 2 - userPrompt->width() / 2;
                auto y = window()->y() + window()->height() / 2 - userPrompt->height() / 2;
                userPrompt->move(x, y);
                userPrompt->show();
                connect(userPrompt, &UserPromptDialog::accepted, this, [this]
                        {
                            fallback(BRFallbackMethod::BR_FALLBACK_METHOD_LAST);
                        });
            });

    connect(m_dbusProxy, &BRDbusProxy::HomeFreeSpaceRatioLower, this, [this](const QString &spaceRatio)
            {
                // 家目录可用空间小于10%告警
                KLOG_WARNING() << "home free space less than 10% , ratio : " << spaceRatio;
                if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
                {
                    Notify::NOTIFY_WARN(tr("Home free space less than 10%.").toUtf8());
                }
            });
    connect(m_dbusProxy, &BRDbusProxy::RootFreeSpaceRatioLower, this, [this](const QString &spaceRatio)
            {
                // 根目录可用空间小于10%告警
                KLOG_WARNING() << "root free space less than 10% , ratio : " << spaceRatio;
                if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
                {
                    Notify::NOTIFY_WARN(tr("Root free space less than 10%.").toUtf8());
                }
            });
    connect(m_dbusProxy, &BRDbusProxy::CpuAverageLoadRatioHigher, this, [this](const QString &loadRatio)
            {
                // cpu单核五分钟平均负载大于1告警
                KLOG_WARNING() << "The average load of a single core CPU exceeds 1 , ratio : " << loadRatio;
                if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
                {
                    Notify::NOTIFY_WARN(tr("The average load of a single core CPU exceeds 1.").toUtf8())
                }
            });

    connect(m_dbusProxy, &BRDbusProxy::MemoryAbnormal, this, [this](const QString &ratio)
            {
                // 内存不足10%告警
                KLOG_WARNING() << "Memory space remaining " << ratio << ", below 10%";
                if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
                {
                    Notify::NOTIFY_WARN(tr("Memory space less than 10%.").toUtf8())
                }
            });
}

void BRSettingPage::initUI()
{
    // 样式
    QFile file(STYLE_PATH);
    if (file.open(QIODevice::ReadOnly))
    {
        QString windowStyle = file.readAll();
        setStyleSheet(windowStyle);
    }
    else
    {
        KLOG_WARNING() << "Failed to open file " << STYLE_PATH;
    }

    if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
    {
        m_ui->m_openNotify->setChecked(true);
        m_ui->m_closeNotify->setChecked(false);
    }
    else
    {
        m_ui->m_openNotify->setChecked(false);
        m_ui->m_closeNotify->setChecked(true);
    }
    if (m_dbusProxy->resource_monitor() == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN)
    {
        m_ui->m_openMonitor->setChecked(true);
        m_ui->m_closeMonitor->setChecked(false);
    }
    else
    {
        m_ui->m_openMonitor->setChecked(false);
        m_ui->m_closeMonitor->setChecked(true);
    }
    m_ui->m_timeSpinBox->setValue(m_dbusProxy->time_scan());
}

void BRSettingPage::updateProgressInfo(KS::BR::ProgressInfo &progressInfo)
{
    for (auto categories : m_categories)
    {
        for (auto reinforcementItem : categories->getReinforcementItem())
        {
            auto state = reinforcementItem->getState();
            if (state == BR_REINFORCEMENT_STATE_SCAN_DONE ||
                state == BR_REINFORCEMENT_STATE_REINFORCE_DONE ||
                (state & BR_REINFORCEMENT_STATE_SAFE) == BR_REINFORCEMENT_STATE_SAFE)
            {
                progressInfo.successCount += 1;
            }
            else if (state == BR_REINFORCEMENT_STATE_UNKNOWN ||
                     state == BR_REINFORCEMENT_STATE_SCAN_ERROR ||
                     state == BR_REINFORCEMENT_STATE_REINFORCE_ERROR ||
                     (state & BR_REINFORCEMENT_STATE_UNSAFE) == BR_REINFORCEMENT_STATE_UNSAFE)
            {
                progressInfo.failureCount += 1;
            }
        }
    }
}

QString BRSettingPage::getTitle()
{
    return tr("Baseline reinforcement");
}

void BRSettingPage::timedScanSettings(int hours)
{
    m_dbusProxy->SetTimeScan(hours);
    if (hours == 0)
    {
        m_timedScan->stop();
        if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
        {
            Notify::NOTIFY_INFO(tr("Scheduled scanning task has been closed!").toUtf8());
        }
        return;
    }

    // 每hours小时扫描一次
    if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
    {
        Notify::NOTIFY_INFO(QString(tr("Scheduled scanning task has been started, every interval %1 scan once every hour.")).arg(hours).toUtf8());
    }
    m_timedScan->start(hours * 1000 * 3600);
}

void BRSettingPage::scan()
{
    m_categories.clear();
    m_progressInfo = {};
    // 获取加固项信息
    auto reply = m_dbusProxy->GetCategories();
    reply.waitForFinished();
    KS::BR::Utils::getDefault()->jsonParsing(reply.value().toUtf8(), m_categories);
    KS::BR::Utils::getDefault()->ssrReinforcements(m_dbusProxy->GetReinforcements().value(), m_categories);

    // 断开scan进程连接
    disconnect(m_dbusProxy, &BRDbusProxy::ScanProgress, 0, 0);
    // 进行一次扫描 仅获取扫描结果
    connect(m_dbusProxy, &BRDbusProxy::ScanProgress, this, [this](const QString &jobResult)
            {
                InvalidData invalidData = {};
                KS::BR::Utils::getDefault()->ssrJobResult(jobResult, m_progressInfo, m_categories, invalidData);
            });
    // 监听进程完成后
    disconnect(m_dbusProxy, &BRDbusProxy::ProgressFinished, 0, 0);
    connect(m_dbusProxy, &BRDbusProxy::ProgressFinished, this, [this]
            {
                disconnect(m_dbusProxy, &BRDbusProxy::ProgressFinished, 0, 0);
                updateProgressInfo(m_progressInfo);
                if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
                {
                    Notify::NOTIFY_INFO(QString(tr("Timed scan finished, Scaned %1, %2 conform, %3 inconform!"))
                                            .arg(m_progressInfo.failureCount + m_progressInfo.successCount)
                                            .arg(m_progressInfo.successCount)
                                            .arg(m_progressInfo.failureCount)
                                            .toUtf8());
                }
                KLOG_INFO() << "Timed scan finied, Scaned "
                            << m_progressInfo.failureCount + m_progressInfo.successCount
                            << ", "
                            << m_progressInfo.successCount
                            << " conform, "
                            << m_progressInfo.failureCount
                            << "inconform!";
            });

    QStringList scanStr;
    for (auto categories : m_categories)
    {
        for (auto reinforcementItem : categories->getReinforcementItem())
        {
            scanStr << reinforcementItem->getName();
        }
    }
    auto replyScan = m_dbusProxy->Scan(scanStr);
    CHECK_ERROR_FOR_DBUS_REPLY(replyScan);
}

void BRSettingPage::setMonitorStatus(bool isOpen)
{
    m_dbusProxy->SetResourceMonitorSwitch(isOpen ? BR_RESOURCE_MONITOR_OPEN : BR_RESOURCE_MONITOR_CLOSE);

    RETURN_IF_TRUE(m_dbusProxy->notification_status() == BR_NOTIFICATION_STATUS_CLOSE);
    Notify::NOTIFY_INFO(isOpen ? tr("Open resource monitoring.").toUtf8() : tr("Close resource monitoring.").toUtf8());
    KLOG_DEBUG() << QString(isOpen ? "Open resource monitoring" : "Close resource monitoring");
}

void BRSettingPage::fallback(int status)
{
    auto reply = m_dbusProxy->Fallback(BRFallbackMethod(status));
    CHECK_ERROR_FOR_DBUS_REPLY_AND_RETURN(reply);

    connect(m_dbusProxy, &BRDbusProxy::FallbackFinished, this, [this]
            {
                POPUP_MESSAGE_DIALOG(tr("Fallback finished!"));
                disconnect(m_dbusProxy, &BRDbusProxy::FallbackFinished, 0, 0);
            });
}
}  // namespace BR
}  // namespace KS
