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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#include "dbus.h"
#include <daemon-authentication-i.h>
#include <daemon-log-i.h>
#include <kylin-license/license-i.h>
#include <libaudit.h>
#include <qt5-log-i.h>
#include <unistd.h>
#include <QPair>
#include <QTimer>
#include <fstream>
#include <iostream>
#include "br_adaptor.h"
#include "categories.h"
#include "configuration.h"
#include "include/ssr-marcos.h"
#include "job-dispatcher.h"
#include "lib/base/error.h"
#include "lib/base/report.h"
#include "lib/dbus/dbus-helper.h"
#include "plugins.h"
#include "utils.h"

namespace KS
{
namespace BR
{
// 一分钟
#define RESOURCEMONITORMS 1000 * 60 * 1

#define CUSTOM_RA_STRATEGY_FILEPATH SSR_BR_INSTALL_DATADIR "/br-custom-ra-strategy.xml"

static int _audit_log(int type, int rc, const char* op)
{
    int audit_fd;

    audit_fd = audit_open();
    if (audit_fd < 0)
    {
        /* You get these error codes only when the kernel doesn't have
         * audit compiled in. */
        if (errno == EINVAL || errno == EPROTONOSUPPORT ||
            errno == EAFNOSUPPORT)
            return 0;

        KLOG_WARNING("audit_open() failed: %d", LOG_CRIT);
        return -1;
    }

    rc = audit_log_acct_message(audit_fd, type, NULL, op,
                                NULL, -1, NULL, NULL, NULL, rc == 0);
    if (rc == -EPERM && geteuid() != 0)
    {
        rc = 0;
    }

    audit_close(audit_fd);

    return rc < 0 ? -1 : 0;
}

BRDBus::BRDBus(Configuration* configuration,
               Categories* categories,
               Plugins* plugins,
               JobDispatcher* jobDispatcher,
               QObject* parent)
    : QObject(parent),
      m_resourceMonitorTimer(nullptr),
      m_configuration(configuration),
      m_categories(categories),
      m_plugins(plugins),
      m_jobDispatcher(jobDispatcher),
      m_isScanFlag(true),
      m_reinforceTimer(nullptr)
{
    m_dbus = new BRAdaptor(this);
    m_resourceMonitor = new ResourceMonitor(this);
    m_reinforceTimer = new QTimer(this);
    m_reinforceTimer->setInterval(100);
}

BRDBus::~BRDBus()
{
    if (this->m_resourceMonitorTimer)
    {
        m_resourceMonitorTimer->stop();
        QObject::disconnect(this->m_resourceMonitorTimer, SIGNAL(QTimer::timeout()), this, SLOT(BRDBus::setResourceMonitor()));
        delete this->m_resourceMonitorTimer;
    }
}

void BRDBus::init()
{
    QDBusConnection dbusConnection = QDBusConnection::systemBus();
    if (!dbusConnection.registerObject(BR_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "Register Service error:" << dbusConnection.lastError().message();
        return;
    }

    if (m_configuration->getResourceMonitorStatus() == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN)
    {
        m_resourceMonitorTimer = new QTimer(this);
        QObject::connect(this->m_resourceMonitorTimer, &QTimer::timeout, this, &BRDBus::setResourceMonitor);
        m_resourceMonitorTimer->start(RESOURCEMONITORMS);
    }

    QObject::connect(this->m_resourceMonitor, &ResourceMonitor::homeFreeSpaceRatio_,
                     this, &BRDBus::homeFreeSpaceRatio);
    QObject::connect(this->m_resourceMonitor, &ResourceMonitor::rootFreeSpaceRatio_,
                     this, &BRDBus::rootFreeSpaceRatio);
    QObject::connect(this->m_resourceMonitor, &ResourceMonitor::cpuAverageLoadRatio_,
                     this, &BRDBus::cpuAverageLoadRatio);
    QObject::connect(this->m_resourceMonitor, &ResourceMonitor::memoryRemainingRatio_, this, &BRDBus::memoryRemainingRatio);

    connect(m_plugins, &Plugins::reinforcementsChanged, [this]()
            {
                Q_EMIT ReinforcementsChanged();
            });
}

uint BRDBus::notificationStatus() const
{
    return m_configuration->getNotificationStatus();
}

uint BRDBus::resourceMonitor() const
{
    return m_configuration->getResourceMonitorStatus();
}

uint BRDBus::standardType() const
{
    return m_configuration->getStandardType();
}

uint BRDBus::strategyType() const
{
    return m_configuration->getStrategyType();
}

uint BRDBus::timeScan() const
{
    return m_configuration->getTimeScan();
}

QString BRDBus::version() const
{
    return PROJECT_VERSION;
}

CHECK_AUTH_WITH_1ARGS(BRDBus, Reinforce, reinforce, SSR_POLICY_ADMINISTRATION, {ACCOUNT_ROLE_SYSADMIN}, const QStringList&);
CHECK_AUTH_WITH_1ARGS(BRDBus, Rollback, rollback, SSR_POLICY_ADMINISTRATION, {ACCOUNT_ROLE_SYSADMIN}, const uint32_t&);

void BRDBus::SetStandardType(const uint32_t& standardType)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    if (standardType >= BRStandardType::BR_STANDARD_TYPE_LAST)
    {
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set standard type to %1.")
                          .arg(standardType == BR_STANDARD_TYPE_SYSTEM ? tr("system") : tr("custom")),
                      calledUniqueName);
        sendErrorReply(QDBusError::InvalidArgs, SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_STANDARD_TYPE_INVALID));
        return;
    }

    RETURN_IF_TRUE(standardType == this->m_configuration->getStandardType())

    if (!this->m_configuration->setStandardType(BRStandardType(standardType)))
    {
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set standard type to %1.")
                          .arg(standardType == BR_STANDARD_TYPE_SYSTEM ? tr("system") : tr("custom")),
                      calledUniqueName);
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_SET_STANDARD_TYPE_FAILED));
        return;
    }
    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                    tr("Success to set standard type to %1.")
                        .arg(standardType == BR_STANDARD_TYPE_SYSTEM ? tr("system") : tr("custom")),
                    calledUniqueName);
}

void BRDBus::ImportCustomRS(const QString& encodedStandard)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);

    SSRErrorCode errorCode = SSRErrorCode::SUCCESS;
    if (!this->m_configuration->setCustomRS(encodedStandard, errorCode))
    {
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(errorCode));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to import custom reinforcement standard."),
                      calledUniqueName);
        return;
    }
    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                    tr("Success to import custom reinforcement standard."),
                    calledUniqueName);
}

void BRDBus::SetStrategyType(const uint32_t& strategyType)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    if (strategyType >= BRStrategyType::BR_STRATEGY_TYPE_LAST)
    {
        sendErrorReply(QDBusError::InvalidArgs, SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_STRATEGY_TYPE_INVALID));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set strategy type."),
                      calledUniqueName);
        return;
    }
    if (strategyType == this->m_configuration->getStrategyType())
    {
        SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                        tr("Set strategy type to %1.")
                            .arg(strategyType == BR_STRATEGY_TYPE_SYSTEM ? tr("system") : tr("custom")),
                        calledUniqueName);
        return;
    }

    if (!this->m_configuration->setStrategyType(BRStrategyType(strategyType)))
    {
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_SET_STRATEGY_TYPE_FAILED));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set strategy type."),
                      calledUniqueName);
        return;
    }
    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                    tr("Set strategy type to %1.")
                        .arg(strategyType == BR_STRATEGY_TYPE_SYSTEM ? tr("system") : tr("custom")),
                    calledUniqueName);
}

void BRDBus::SetTimeScan(const uint32_t& timeScan)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);

    if (timeScan == uint32_t(this->m_configuration->getTimeScan()))
    {
        SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                        tr("Set time scan to %1.").arg(timeScan),
                        calledUniqueName);
        return;
    }
    if (!this->m_configuration->setTimeScan(int(timeScan)))
    {
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_SET_TIME_SCAN_FAILED));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set time scan."),
                      calledUniqueName);
        return;
    }
    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                    tr("Set time scan to %1.").arg(timeScan),
                    calledUniqueName);
}

void BRDBus::SetNotificationStatus(const uint32_t& notificationStatus)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);

    if (notificationStatus >= BRNotificationStatus::BR_NOTIFICATION_STATUS_OTHER)
    {
        sendErrorReply(QDBusError::InvalidArgs,
                       SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_NOTIFICATION_STATUS_INVALID));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set notification status."),
                      calledUniqueName);
        return;
    }
    if (notificationStatus == this->m_configuration->getNotificationStatus())
    {
        SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                        tr("Set notification status to %1.").arg(notificationStatus == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN ? tr("open") : tr("close")),
                        calledUniqueName);
        return;
    }

    if (!this->m_configuration->setNotificationStatus(BRNotificationStatus(notificationStatus)))
    {
        sendErrorReply(QDBusError::InternalError,
                       SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_SET_NOTIFICATION_STATUS_FAILED));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set notification status."),
                      calledUniqueName);
        return;
    }

    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                    tr("Set notification status to %1.").arg(notificationStatus == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN ? tr("open") : tr("close")),
                    calledUniqueName);
}

void BRDBus::ImportCustomRA(const QString& encodedStrategy)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    try
    {
        std::ofstream ofs(CUSTOM_RA_STRATEGY_FILEPATH, std::ios_base::out);
        ofs << encodedStrategy.toStdString();
        ofs.close();
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to import custom reinforcement strategy."),
                      calledUniqueName);
        return;
    }
    if (!m_configuration->checkRaStrategy())
    {
        remove(CUSTOM_RA_STRATEGY_FILEPATH);
        // 不知道选选哪个错误码，所以选择了 ERROR_FAILED
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_FAILED));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to import custom reinforcement strategy."),
                      calledUniqueName);
        return;
    }
    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                    tr("Import custom reinforcement strategy."),
                    calledUniqueName);
}

void BRDBus::SetCheckBox(const QString& reinforcementName, const bool& checkboxStatus)
{
    m_configuration->setRaCheckbox(reinforcementName, checkboxStatus);
}

void BRDBus::SetResourceMonitorSwitch(const uint32_t& resourceMonitor)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    if (resourceMonitor >= BRResourceMonitor::BR_RESOURCE_MONITOR_OTHER)
    {
        sendErrorReply(QDBusError::InvalidArgs, SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_RESOURCE_MONITOR_INVALID));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set resource monitor switch."),
                      calledUniqueName);
        return;
    }

    if (resourceMonitor == this->m_configuration->getResourceMonitorStatus())
    {
        SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                        tr("Set resource monitor switch to %1.").arg(resourceMonitor == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN ? tr("open") : tr("close")),
                        calledUniqueName);
        return;
    }

    if (!this->m_configuration->setResourceMonitorStatus(BRResourceMonitor(resourceMonitor)))
    {
        sendErrorReply(QDBusError::InvalidArgs, SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_SET_RESOURCE_MONITOR_FAILED));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set resource monitor switch."),
                      calledUniqueName);
        return;
    }

    m_resourceMonitorTimer->stop();
    QObject::disconnect(this->m_resourceMonitorTimer, SIGNAL(timeout()),
                        this, SLOT(BRDBus::setResourceMonitor()));
    if (BRResourceMonitor(resourceMonitor) == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN)
    {
        this->m_resourceMonitorTimer = new QTimer(this);
        QObject::connect(this->m_resourceMonitorTimer, &QTimer::timeout,
                         this, &BRDBus::setResourceMonitor);
        this->m_resourceMonitorTimer->start(RESOURCEMONITORMS);
    }

    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                    tr("Set resource monitor switch to %1.").arg(resourceMonitor == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN ? tr("open") : tr("close")),
                    calledUniqueName);
}

QString BRDBus::GetCategories()
{
    QJsonObject jsonObject;
    QJsonArray jsonArray;

    auto categories = this->m_categories->getCategories();

    jsonObject["itemCount"] = int32_t(categories.size());
    for (uint32_t i = 0; i < static_cast<uint32_t>(categories.size()); ++i)
    {
        auto category = categories[static_cast<int>(i)];

        QJsonObject jsonObjectTmp;
        jsonObjectTmp["name"] = category->name;
        jsonObjectTmp["label"] = category->label;
        jsonObjectTmp["description"] = category->description;
        jsonObjectTmp["icon_name"] = category->icon_name;

        jsonArray.append(jsonObjectTmp);
    }
    jsonObject["items"] = jsonArray;
    return QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
}

QString BRDBus::GetReinforcements()
{
    std::ostringstream ostringStream;
    Protocol::Reinforcements protocolReinforcements;

    auto reinforcements = this->m_plugins->getReinforcements();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        auto& rsReinforcement = (*iter)->getRs();
        protocolReinforcements.reinforcement().push_back(rsReinforcement);
    }

    try
    {
        Protocol::br_reinforcements(ostringStream, protocolReinforcements);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_GEN_REINFORCEMENT_FAILED));
    }
    return QString::fromStdString(ostringStream.str());
}

void BRDBus::ResetReinforcements()
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);

    this->m_configuration->delAllCustomRA();
    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                    tr("Reset all reinforcement parameters."),
                    calledUniqueName);
}

QString BRDBus::GetReinforcement(const QString& name)
{
    std::ostringstream ostringStream;
    auto reinforcement = this->m_plugins->getReinforcement(name);
    if (!reinforcement)
    {
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_REINFORCEMENT_NOTFOUND));
    }
    auto& rsReinforcement = reinforcement->getRs();

    try
    {
        Protocol::br_reinforcement(ostringStream, rsReinforcement);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_GEN_REINFORCEMENT_FAILED));
    }
    return QString::fromStdString(ostringStream.str());
}

void BRDBus::SetReinforcement(const QString& reinforcement)
{
    if (m_configuration->getStrategyType() == BRStrategyType::BR_STRATEGY_TYPE_SYSTEM)
    {
        sendErrorReply(QDBusError::NotSupported,
                       SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_NEED_CUSTOM_STRATEGY_TYPE));
        return;
    }

    try
    {
        std::istringstream istringStream(reinforcement.toStdString());
        auto brReinforcement = Protocol::br_reinforcement(istringStream, xml_schema::Flags::dont_validate);
        if (!this->m_configuration->setCustomRA(*brReinforcement.get()))
        {
            sendErrorReply(QDBusError::InternalError,
                           SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_SET_REINFORCEMENT_FAILED));
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError,
                       SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_SET_REINFORCEMENT_FAILED));
    }
}

void BRDBus::SetReinforcements(const QString& reinforcements)
{
    if (m_configuration->getStrategyType() == BRStrategyType::BR_STRATEGY_TYPE_SYSTEM)
    {
        sendErrorReply(QDBusError::NotSupported,
                       SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_NEED_CUSTOM_STRATEGY_TYPE));
        return;
    }

    try
    {
        std::istringstream istringStream(reinforcements.toStdString());
        auto brReinforcements = Protocol::br_reinforcements(istringStream, xml_schema::Flags::dont_validate);
        for (const auto& brReinforcement : brReinforcements->reinforcement())
        {
            if (!this->m_configuration->setCustomRA(brReinforcement))
            {
                sendErrorReply(QDBusError::InternalError,
                               SSR_ERROR2STR(SSRErrorCode::ERROR_PLUGIN_BR_SET_REINFORCEMENT_FAILED));
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError,
                       SSR_ERROR2STR(SSRErrorCode::ERROR_DAEMON_SET_REINFORCEMENT_FAILED));
    }
}

bool BRDBus::ResetReinforcement(const QString& name)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);

    if (m_configuration->getStrategyType() == BRStrategyType::BR_STRATEGY_TYPE_SYSTEM)
    {
        sendErrorReply(QDBusError::NotSupported,
                       SSR_ERROR2STR(SSRErrorCode::ERROR_BR_NEED_CUSTOM_STRATEGY_TYPE));
        return false;
    }

    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                    tr("Reset reinforcement parameters. name is %1.").arg(name),
                    calledUniqueName);

    return this->m_configuration->delCustomRA(name);
}

void BRDBus::Scan(const QStringList& names)
{
    if (!validateReinforcementNames(names))
    {
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_COMMON_INVALID_ARGS));
        return;
    }

    if (m_jobDispatcher->getState() != BRDispatchState::BR_DISPATCH_STATE_IDLE)
    {
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_BR_JOB_IS_RUNNING));
        return;
    }

    m_scanUniqueName = message().service();
    if (!m_jobDispatcher->scan(names))
    {
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_FAILED));
        return;
    }

    connect(m_jobDispatcher, &JobDispatcher::scanProgress, this, &BRDBus::processScanProgress);
    connect(m_jobDispatcher, &JobDispatcher::scanFinished, this, &BRDBus::processScanFinished);
}

QString BRDBus::GetScanResult()
{
    auto scanResult = m_jobDispatcher->getScanResult();
    std::ostringstream ostringStream;
    Protocol::br_job_result(ostringStream, scanResult);
    return QString(ostringStream.str().c_str());
}

void BRDBus::reinforce(const QDBusMessage& message, const QStringList& names)
{
    if (!validateReinforcementNames(names))
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_COMMON_INVALID_ARGS, message);
    }

    if (m_jobDispatcher->getState() != BRDispatchState::BR_DISPATCH_STATE_IDLE)
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_BR_JOB_IS_RUNNING, message);
    }

    m_reforceUniqueName = message.service();
    if (!m_jobDispatcher->reinforce(names))
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_FAILED, message);
    }

    connect(m_jobDispatcher, &JobDispatcher::reinforceProgress, this, &BRDBus::processReinforceProgress);
    connect(m_jobDispatcher, &JobDispatcher::reinforceFinished, this, &BRDBus::processReinforceFinished);
    QDBusConnection::systemBus().send(message.createReply());
}

QString BRDBus::GetReinforceResult()
{
    auto reinforceResult = m_jobDispatcher->getReinforceResult();
    std::ostringstream ostringStream;
    Protocol::br_job_result(ostringStream, reinforceResult);
    return QString(ostringStream.str().c_str());
}

uint BRDBus::GetDispatchStatus()
{
    return m_jobDispatcher->getState();
}

void BRDBus::Cancel(const qlonglong& jobID)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);

    if (!m_jobDispatcher->cancel(jobID))
    {
        sendErrorReply(QDBusError::Failed, SSR_ERROR2STR(SSRErrorCode::ERROR_FAILED));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to cancel progress."),
                      calledUniqueName);
    }
    else
    {
        SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT,
                        tr("Cancel. job id: %1.").arg(jobID),
                        calledUniqueName);
    }
}

void BRDBus::ExportStrategy(bool operationResult)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    SSR_LOG(LogType::BASELINE_REINFORCEMENT,
            tr("Export strategy."),
            operationResult,
            calledUniqueName);
}
void BRDBus::GenerateReport(bool operationResult)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    SSR_LOG(LogType::BASELINE_REINFORCEMENT,
            tr("Export report."),
            operationResult,
            calledUniqueName);
}

void BRDBus::ExportReport(const QString& savePath)
{
    if (m_jobDispatcher->getState() != BRDispatchState::BR_DISPATCH_STATE_IDLE)
    {
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_BR_JOB_IS_RUNNING));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to export report."),
                      message().service());
        return;
    }

    if (!m_jobDispatcher->scanAll())
    {
        sendErrorReply(QDBusError::InternalError, SSR_ERROR2STR(SSRErrorCode::ERROR_FAILED));
        SSR_LOG_ERROR(LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to export report."),
                      message().service());
        return;
    }

    m_exportReportConnection = connect(m_jobDispatcher, &JobDispatcher::scanFinished,
                                       std::bind(&BRDBus::exportReport, this, savePath));
}

void BRDBus::rollback(const QDBusMessage& message, const uint32_t& snapshotStatus)
{
    // 已经在加固则返回错误
    if (m_jobDispatcher->getState() != BRDispatchState::BR_DISPATCH_STATE_IDLE)
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_BR_JOB_IS_RUNNING, message);
    }

    m_rollbackUniqueName = message.service();
    if (!m_jobDispatcher->rollback(snapshotStatus))
    {
        DBUS_ERROR_REPLY_AND_RETURN(SSRErrorCode::ERROR_FAILED, message);
    }

    connect(m_jobDispatcher, &JobDispatcher::rollbackFinished, this, &BRDBus::processRollbackFinished);
    QDBusConnection::systemBus().send(message.createReply());
}

void BRDBus::processScanProgress(const QString& progress)
{
    Q_EMIT ScanProgress(progress);
}

void BRDBus::processReinforceProgress(const QString& progress)
{
    Q_EMIT ReinforceProgress(progress);
}

bool BRDBus::setResourceMonitor()
{
    if (m_configuration->getResourceMonitorStatus() == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN)
        m_resourceMonitor->startMonitor();
    else if (m_configuration->getResourceMonitorStatus() == BRResourceMonitor::BR_RESOURCE_MONITOR_CLOSE)
        m_resourceMonitor->closeMonitor();
    else
        m_resourceMonitor->startMonitor();
    return true;
}

void BRDBus::processScanFinished()
{
    disconnect(m_jobDispatcher, &JobDispatcher::scanProgress, this, &BRDBus::processScanProgress);
    disconnect(m_jobDispatcher, &JobDispatcher::scanFinished, this, &BRDBus::processScanFinished);
    // 记录扫描日志
    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT, tr("Scan finished."), m_scanUniqueName);
    emit ProgressFinished();
    emit ScanFinished();
}

void BRDBus::processReinforceFinished()
{
    disconnect(m_jobDispatcher, &JobDispatcher::reinforceProgress, this, &BRDBus::processReinforceProgress);
    disconnect(m_jobDispatcher, &JobDispatcher::reinforceFinished, this, &BRDBus::processReinforceFinished);
    // 记录加固完成日志
    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT, tr("Reinforcement finished."), m_reforceUniqueName);
    emit ProgressFinished();
    emit ReinforceFinished();
}

void BRDBus::processRollbackFinished()
{
    disconnect(m_jobDispatcher, &JobDispatcher::rollbackFinished, this, &BRDBus::processRollbackFinished);
    SSR_LOG_SUCCESS(LogType::BASELINE_REINFORCEMENT, tr("Rollback finished."), m_rollbackUniqueName);
    emit RollbackFinished();
}

void BRDBus::exportReport(const QString& savePath)
{
    disconnect(m_exportReportConnection);

    CategoryVec categoryVec = m_categories->getCategories();
    QMap<QString, QPair<int, int>> categoryMap;
    // 动态翻译
    static const QMap<QString, const char*> categoryTR = {
        {"config", QT_TR_NOOP("config")},
        {"network", QT_TR_NOOP("network")},
        {"audit", QT_TR_NOOP("audit")},
        {"external", QT_TR_NOOP("external")}};

    for (auto& category : categoryVec)
    {
        categoryMap[category->name] = qMakePair(0, 0);
    }

    QList<QStringList> tabelData;
    tabelData.push_back({tr("Test Item"), tr("Result")});

    QList<QStringList> tabelDataConformity;
    QList<QStringList> tabelDataInconformity;

    auto scanJobResult = m_jobDispatcher->getScanResult();
    for (auto& reinforcementResult : scanJobResult.reinforcement())
    {
        QString name = QString::fromStdString(reinforcementResult.name());
        QString label = this->m_plugins->getReinforcement(name)->getLabel();

        auto state = reinforcementResult.state();
        QString stateStr;
        if ((state & BR_REINFORCEMENT_STATE_SAFE) == 1)
        {
            stateStr = QString(tr("Conformity"));
            tabelDataConformity.push_back({noop2Translate(label), stateStr});
        }
        else
        {
            stateStr = QString(tr("Inconformity"));
            tabelDataInconformity.push_back({noop2Translate(label), stateStr});
        }

        auto category = this->m_plugins->getReinforcement(name)->getCategoryName();

        if (categoryMap.contains(category))
        {
            (state & BR_REINFORCEMENT_STATE_SAFE) == 1 ? categoryMap[category].first++ : categoryMap[category].second++;
        }
    }

    tabelData.append(tabelDataInconformity);
    tabelData.append(tabelDataConformity);

    QList<QPair<QString, QString>> homeExtraData;
    auto iter = categoryMap.begin();
    while (iter != categoryMap.end())
    {
        QString key = categoryTR.contains(iter.key()) ? tr(categoryTR[iter.key()]) : iter.key();

        homeExtraData.push_back({key + ":", tr("total:%1 conformity:%2 conformity:%3").arg(iter.value().first + iter.value().second).arg(iter.value().first).arg(iter.value().second)});
        iter++;
    }

    QString failedReason = Report::genReport(savePath, homeExtraData, tr("test information"), tabelData);

    emit ExportReportFinished(failedReason);
}

void BRDBus::homeFreeSpaceRatio(float spaceRatio)
{
    // 家目录可用空间小于10%告警
    float homeSpa = 0.1;
    RETURN_IF_TRUE(spaceRatio >= homeSpa)

    KLOG_WARNING() << "home free space less than 10%. homeFreeSpaceRatio " << spaceRatio;
    _audit_log(1101, -1, "home free space less than 10%.");
    emit HomeFreeSpaceRatioLower(QString(std::to_string(spaceRatio).c_str()));
}

void BRDBus::rootFreeSpaceRatio(float spaceRatio)
{
    // 根目录可用空间小于10%告警
    float rootSpa = 0.1;
    RETURN_IF_TRUE(spaceRatio >= rootSpa)

    KLOG_WARNING() << "root free space less than 10%. rootFreeSpaceRatio " << spaceRatio;
    _audit_log(1101, -1, "root free space less than 10%.");
    emit RootFreeSpaceRatioLower(QString(std::to_string(spaceRatio).c_str()));
}

void BRDBus::cpuAverageLoadRatio(float loadRatio)
{
    // cpu单核五分钟平均负载大于1告警
    float cpuLoad = 1;
    RETURN_IF_TRUE(loadRatio < cpuLoad)

    KLOG_WARNING() << "The average load of a single core CPU exceeds 1. The average load ratio is " << loadRatio;
    _audit_log(1101, -1, "The average load of a single core CPU exceeds 1.");
    emit CpuAverageLoadRatioHigher(QString(std::to_string(loadRatio).c_str()));
}

void BRDBus::memoryRemainingRatio(float memoryRatio)
{
    // memory ratio 小于10% 告警
    RETURN_IF_TRUE(memoryRatio >= 0.1)
    KLOG_WARNING("Memory space remaining %f, below 10 percent", memoryRatio);
    _audit_log(1101, -1, "Memory space less than 10%");
    MemoryAbnormal(QString(std::to_string(memoryRatio).c_str()));
}

bool BRDBus::validateReinforcementNames(const QStringList& reinforcementNames)
{
    for (const auto& reinforcementName : reinforcementNames)
    {
        RETURN_VAL_IF_FALSE(m_plugins->getReinforcement(reinforcementName), false);
    }
    return true;
}

QString BRDBus::python2Translate(const QString& souceTxt)
{
    return qApp->translate("python", souceTxt.toUtf8());
}

QString BRDBus::noop2Translate(const QString& souceTxt)
{
    auto tmpSouce = souceTxt;
    auto tmpList = tmpSouce.split("\"");
    QStringList translateList;
    for (auto key : tmpList)
    {
        if (key.isEmpty() || key == "," || key == ", " || key == "QT_TRANSLATE_NOOP(" || key == "QT_TRANSLATE_NOOP_UTF8(" || key == ")")
            continue;
        key.remove(QRegExp("^ +\\s*"));
        translateList << key;
    }

    if (translateList.size() != 2)
        return souceTxt;
    return qApp->translate(translateList[0].toUtf8(), translateList[1].toUtf8());
}

}  // namespace BR
}  // namespace KS
