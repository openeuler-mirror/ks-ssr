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
#include "baseline-reinforcement.h"
#include <qt5-log-i.h>
#include <QFileDialog>
#include "br_dbus_proxy.h"
#include "include/ssr-i.h"
#include "lib/base/notification-wrapper.h"
#include "src/ui/br/br-i.h"
#include "src/ui/br/reinforcement-items/category.h"
#include "src/ui/br/utils.h"
#include "src/ui/common/user-prompt-dialog.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "ui_baseline-reinforcement.h"

namespace KS
{
namespace Settings
{
BaselineReinforcement::BaselineReinforcement(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::BaselineReinforcement)
{
    m_ui->setupUi(this);

    m_dbusProxy = new BRDbusProxy(SSR_DBUS_NAME,
                                  BR_DBUS_OBJECT_PATH,
                                  QDBusConnection::systemBus(),
                                  this);
    initConnection();
    initUI();
}

BaselineReinforcement::~BaselineReinforcement()
{
    delete m_ui;
}

void BaselineReinforcement::initConnection()
{
    connect(m_ui->m_importStrategy, &QPushButton::clicked, this, &BaselineReinforcement::importStrategy);
    connect(m_ui->m_exportStrategy, &QPushButton::clicked, this, &BaselineReinforcement::exportStrategyClicked);
    connect(m_ui->m_resetAllArgs, &QPushButton::clicked, this, &BaselineReinforcement::resetAllArgsClicked);
    connect(m_ui->m_timeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &BaselineReinforcement::timedScanSettings);
    m_timedScan = new QTimer(this);
    connect(m_timedScan, &QTimer::timeout, this, &BaselineReinforcement::scan);
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
                auto userPrompt = new UserPromptDialog(this);
                userPrompt->setNotifyMessage(tr("Fallback"), tr("Are you sure you want to go back to the initialization state."));
                auto x = window()->x() + window()->width() / 2 - userPrompt->width() / 2;
                auto y = window()->y() + window()->height() / 2 - userPrompt->height() / 2;
                userPrompt->move(x, y);
                userPrompt->show();
                connect(userPrompt, &UserPromptDialog::accepted, this, [this]{
                    fallback(BRFallbackMethod::BR_FALLBACK_METHOD_INITIAL);
                });
            });
    connect(m_ui->m_fallbackPrevious, &QPushButton::clicked, this, [this]
            {
                auto userPrompt = new UserPromptDialog(this);
                userPrompt->setNotifyMessage(tr("Fallback"), tr("Are you sure you want to go back to the previous state."));
                auto x = window()->x() + window()->width() / 2 - userPrompt->width() / 2;
                auto y = window()->y() + window()->height() / 2 - userPrompt->height() / 2;
                userPrompt->move(x, y);
                userPrompt->show();
                connect(userPrompt, &UserPromptDialog::accepted, this, [this]{
                    fallback(BRFallbackMethod::BR_FALLBACK_METHOD_LAST);
                });
            });

    connect(m_dbusProxy, &BRDbusProxy::HomeFreeSpaceRatioLower, this, [this](const QString &spaceRatio)
            {
                // 家目录可用空间小于10%告警
                KLOG_WARNING() << "home free space less than 10% , ratio : " << spaceRatio;
                if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
                {
                    Notify::NOTIFY_WARN(tr("home free space less than 10%").toUtf8());
                }
            });
    connect(m_dbusProxy, &BRDbusProxy::RootFreeSpaceRatioLower, this, [this](const QString &spaceRatio)
            {
                // 根目录可用空间小于10%告警
                KLOG_WARNING() << "root free space less than 10% , ratio : " << spaceRatio;
                if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
                {
                    Notify::NOTIFY_WARN(tr("root free space less than 10%").toUtf8());
                }
            });
    connect(m_dbusProxy, &BRDbusProxy::CpuAverageLoadRatioHigher, this, [this](const QString &loadRatio)
            {
                // cpu单核五分钟平均负载大于1告警
                KLOG_WARNING() << "The average load of a single core CPU exceeds 1 , ratio : " << loadRatio;
                if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
                {
                    Notify::NOTIFY_WARN(tr("The average load of a single core CPU exceeds 1").toUtf8())
                }
            });

    connect(m_dbusProxy, &BRDbusProxy::MemoryAbnormal, this, [this](const QString &ratio)
            {
                // 内存不足10%告警
                KLOG_WARNING() << "Memory space remaining " << ratio << ", below 10%";
                if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
                {
                    Notify::NOTIFY_WARN(tr("Memory space less than 10%").toUtf8())
                }
            });
}

void BaselineReinforcement::initUI()
{
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

void BaselineReinforcement::updateProgressInfo(KS::BR::ProgressInfo &progressInfo)
{
    for (auto categories : m_categories)
    {
        for (auto reinforcementItem : categories->getReinforcementItem())
        {
            auto state = reinforcementItem->getState();
            if (state == BR_REINFORCEMENT_STATE_SCAN_DONE || state == BR_REINFORCEMENT_STATE_REINFORCE_DONE || (state & BR_REINFORCEMENT_STATE_SAFE) == BR_REINFORCEMENT_STATE_SAFE)
            {
                progressInfo.successCount += 1;
            }
            else if (state == BR_REINFORCEMENT_STATE_UNKNOWN || state == BR_REINFORCEMENT_STATE_SCAN_ERROR || state == BR_REINFORCEMENT_STATE_REINFORCE_ERROR || (state & BR_REINFORCEMENT_STATE_UNSAFE) == BR_REINFORCEMENT_STATE_UNSAFE)
            {
                progressInfo.failureCount += 1;
            }
        }
    }
}

uint BaselineReinforcement::getFallbackStatus()
{
    return m_dbusProxy->fallback_status();
}

void BaselineReinforcement::importStrategy()
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Files"), "/", tr("strategy(*.xml)"));
    RETURN_IF_TRUE(fileName.isEmpty())

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        KLOG_WARNING() << "Open files failed!";
        POPUP_MESSAGE_DIALOG(tr("Open files failed!"))
    }
    auto reply = m_dbusProxy->ImportCustomRA(QString::fromUtf8(file.readAll()));
    reply.waitForFinished();
    POPUP_MESSAGE_DIALOG(reply.isError() ? tr("Failed to import strategy file. Please whether the file is valid!") : tr("Import succeeded!"));
    file.close();
}

void BaselineReinforcement::timedScanSettings(int hours)
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

void BaselineReinforcement::scan()
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
                    Notify::NOTIFY_INFO(QString(tr("Timed scan finished, Scaned %1, %2 conform, %3 inconform!")).arg(m_progressInfo.failureCount + m_progressInfo.successCount).arg(m_progressInfo.successCount).arg(m_progressInfo.failureCount).toUtf8());
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

void BaselineReinforcement::setMonitorStatus(bool isOpen)
{
    m_dbusProxy->SetResourceMonitorSwitch(isOpen ? BR_RESOURCE_MONITOR_OPEN : BR_RESOURCE_MONITOR_CLOSE);

    RETURN_IF_TRUE(m_dbusProxy->notification_status() == BR_NOTIFICATION_STATUS_CLOSE);
    Notify::NOTIFY_INFO(isOpen ? tr("Open resource monitoring").toUtf8() : tr("Close resource monitoring").toUtf8());
    KLOG_DEBUG() << QString(isOpen ? "Open resource monitoring" : "Close resource monitoring");
}

void BaselineReinforcement::fallback(int status)
{
    if (m_dbusProxy->fallback_status() == BRFallbackStatus::BR_FALLBACK_STATUS_IN_PROGRESS)
    {
        POPUP_MESSAGE_DIALOG(tr("Fallback is in progress, please wait."));
        return;
    }

    auto reply = m_dbusProxy->SetFallback(BRFallbackMethod(status));
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    if (reply.isError())
    {
        m_dbusProxy->SetFallbackStatus(BRFallbackStatus::BR_FALLBACK_STATUS_NOT_STARTED);
        return;
    }

    disconnect(m_dbusProxy, &BRDbusProxy::ProgressFinished, 0, 0);
    connect(m_dbusProxy, &BRDbusProxy::ProgressFinished, this, [this]
            {
                POPUP_MESSAGE_DIALOG(tr("Fallback finished!"));
                disconnect(m_dbusProxy, &BRDbusProxy::ProgressFinished, 0, 0);
                m_dbusProxy->SetFallbackStatus(BRFallbackStatus::BR_FALLBACK_STATUS_IS_FINISHED);
            });
    m_dbusProxy->SetFallbackStatus(BRFallbackStatus::BR_FALLBACK_STATUS_IN_PROGRESS);
}
}  // namespace Settings
}  // namespace KS
