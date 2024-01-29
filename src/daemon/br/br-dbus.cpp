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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#include "br-dbus.h"
#include <kylin-license/license-i.h>
#include <libaudit.h>
#include <qt5-log-i.h>
#include <unistd.h>
#include <QTimer>
#include <fstream>
#include <iostream>
#include "categories.h"
#include "configuration.h"
#include "include/ssr-marcos.h"
#include "plugins.h"
#include "src/daemon/account/manager.h"
#include "src/daemon/br_adaptor.h"
#include "src/daemon/common/dbus-helper.h"
#include "src/daemon/common/polkit-proxy.h"
#include "src/daemon/log/manager.h"
#include "utils.h"

namespace KS
{
namespace BRDaemon
{
// 一分钟
#define RESOURCEMONITORMS 1000 * 60 * 1

#define JOB_ERROR_STR "error"
#define JOB_RETURN_VALUE "return_value"
#define CUSTOM_RA_STRATEGY_FILEPATH SSR_BR_INSTALL_DATADIR "/br-custom-ra-strategy.xml"
#define RH_BR_OPERATE_DATA_FIRST SSR_BR_INSTALL_DATADIR "/br-rh-first.xml"
#define RH_BR_OPERATE_DATA_LAST SSR_BR_INSTALL_DATADIR "/br-rh-last.xml"

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

BRDBus::BRDBus(QObject* parent)
    : QObject(parent),
      m_resourceMonitorTimer(nullptr),
      m_isScanFlag(true),
      m_isFinishRHWrite(true)
{
    this->m_dbus = new BRAdaptor(this);
    init();
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

BRDBus* BRDBus::m_instance = nullptr;
void BRDBus::globalInit(QObject* parent)
{
    m_instance = new BRDBus(parent);
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

uint BRDBus::fallbackStatus() const
{
    return m_configuration->getFallbackStatus();
}

CHECK_AUTH_WITH_1ARGS(BRDBus, Reinforce, reinforce, SSR_PERMISSION_AUTHENTICATION, const QStringList&);
CHECK_AUTH_WITH_1ARGS(BRDBus, SetFallback, setFallback, SSR_PERMISSION_AUTHENTICATION, const uint32_t&)

void BRDBus::SetStandardType(const uint32_t& standardType)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    if (standardType >= BRStandardType::BR_STANDARD_TYPE_LAST)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set standard type to %1, invalid args")
                          .arg(standardType == BR_STANDARD_TYPE_SYSTEM ? tr("system") : tr("custom")),
                      calledUniqueName);
        sendErrorReply(QDBusError::InvalidArgs, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_STANDARD_TYPE_INVALID));
        return;
    }

    RETURN_IF_TRUE(standardType == this->m_configuration->getStandardType())

    if (!this->m_configuration->setStandardType(BRStandardType(standardType)))
    {
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set standard type to %1, Internal error")
                          .arg(standardType == BR_STANDARD_TYPE_SYSTEM ? tr("system") : tr("custom")),
                      calledUniqueName);
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_STANDARD_TYPE_FAILED));
        return;
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Set standard type to %1")
                        .arg(standardType == BR_STANDARD_TYPE_SYSTEM ? tr("system") : tr("custom")),
                    calledUniqueName);
}

void BRDBus::ImportCustomRS(const QString& encodedStandard)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);

    BRErrorCode errorCode = BRErrorCode::SUCCESS;
    if (!this->m_configuration->setCustomRs(encodedStandard, errorCode))
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(errorCode));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to import custom reinforce standard."),
                      calledUniqueName);
        return;
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Import custom reinforce standard. encoded standard: %1").arg(encodedStandard),
                    calledUniqueName);
}

void BRDBus::SetStrategyType(const uint32_t& strategyType)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    if (strategyType >= BRStrategyType::BR_STRATEGY_TYPE_LAST)
    {
        sendErrorReply(QDBusError::InvalidArgs, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_STRATEGY_TYPE_INVALID));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set strategy type"),
                      calledUniqueName);
        return;
    }
    if (strategyType == this->m_configuration->getStrategyType())
    {
        SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                        tr("Set strategy type to %1")
                            .arg(strategyType == BR_STRATEGY_TYPE_SYSTEM ? tr("system") : tr("custom")),
                        calledUniqueName);
        return;
    }

    if (!this->m_configuration->setStrategyType(BRStrategyType(strategyType)))
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_STRATEGY_TYPE_FAILED));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set strategy type"),
                      calledUniqueName);
        return;
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Set strategy type to %1")
                        .arg(strategyType == BR_STRATEGY_TYPE_SYSTEM ? tr("system") : tr("custom")),
                    calledUniqueName);
}

void BRDBus::SetTimeScan(const uint32_t& timeScan)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);

    if (timeScan == uint32_t(this->m_configuration->getTimeScan()))
    {
        SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                        tr("Set time scan to %1").arg(timeScan),
                        calledUniqueName);
        return;
    }
    if (!this->m_configuration->setTimeScan(int(timeScan)))
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_TIME_SCAN_FAILED));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set time scan"),
                      calledUniqueName);
        return;
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Set time scan to %1").arg(timeScan),
                    calledUniqueName);
}

void BRDBus::SetNotificationStatus(const uint32_t& notificationStatus)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);

    if (notificationStatus >= BRNotificationStatus::BR_NOTIFICATION_STATUS_OTHER)
    {
        sendErrorReply(QDBusError::InvalidArgs,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_NOTIFICATION_STATUS_INVALID));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set notification status"),
                      calledUniqueName);
        return;
    }
    if (notificationStatus == this->m_configuration->getNotificationStatus())
    {
        SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                        tr("Set notification status to %1").arg(notificationStatus == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN ? tr("open") : tr("close")),
                        calledUniqueName);
        return;
    }

    if (!this->m_configuration->setNotificationStatus(BRNotificationStatus(notificationStatus)))
    {
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_NOTIFICATION_STATUS_FAILED));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set notification status"),
                      calledUniqueName);
        return;
    }

    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Set notification status to %1").arg(notificationStatus == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN ? tr("open") : tr("close")),
                    calledUniqueName);
}

void BRDBus::SetFallbackStatus(const uint32_t& fallbackStatus)
{
    if (fallbackStatus > BRFallbackStatus::BR_FALLBACK_STATUS_IS_FINISHED)
    {
        sendErrorReply(QDBusError::InvalidArgs,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_FALLBACK_STATUS_INVALID));
        return;
    }
    RETURN_IF_TRUE(fallbackStatus == this->m_configuration->getFallbackStatus());

    if (!this->m_configuration->setFallbackStatus(BRFallbackStatus(fallbackStatus)))
    {
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_FALLBACK_STATUS_FAILED));
        return;
    }
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
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to import custom reinforce strategy. error msg: ") + e.what(),
                      calledUniqueName);
        return;
    }
    if (!m_configuration->checkRaStrategy())
    {
        remove(CUSTOM_RA_STRATEGY_FILEPATH);
        // 不知道选选哪个错误码，所以选择了 ERROR_FAILED
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_FAILED));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to import custom reinforce strategy."),
                      calledUniqueName);
        return;
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Import custom reinforce strategy."),
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
        sendErrorReply(QDBusError::InvalidArgs, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_RESOURCE_MONITOR_INVALID));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set resource monitor switch"),
                      calledUniqueName);
        return;
    }

    if (resourceMonitor == this->m_configuration->getResourceMonitorStatus())
    {
        SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                        tr("Set resource monitor switch to %1").arg(resourceMonitor == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN ? tr("open") : tr("close")),
                        calledUniqueName);
        return;
    }

    if (!this->m_configuration->setResourceMonitorStatus(BRResourceMonitor(resourceMonitor)))
    {
        sendErrorReply(QDBusError::InvalidArgs, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_RESOURCE_MONITOR_FAILED));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set resource monitor switch"),
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

    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Set resource monitor switch to %1").arg(resourceMonitor == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN ? tr("open") : tr("close")),
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

QString BRDBus::GetRS()
{
    std::ostringstream ostringStream;
    auto rs = this->m_configuration->getRs();

    if (!rs)
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_GET_RS_FAILED));
    }

    try
    {
        Protocol::br_rs(ostringStream, *rs.get());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_GET_RS_FAILED));
    }
    return QString::fromStdString(ostringStream.str());
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
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENT_FAILED));
    }
    return QString::fromStdString(ostringStream.str());
}

void BRDBus::ResetReinforcements()
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);

    this->m_configuration->delAllCustomRa();
    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Reset all reinforcement parameters."),
                    calledUniqueName);
}

QString BRDBus::GetReinforcement(const QString& name)
{
    std::ostringstream ostringStream;
    auto reinforcement = this->m_plugins->getReinforcement(name);
    if (!reinforcement)
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND));
    }
    auto& rsReinforcement = reinforcement->getRs();

    try
    {
        Protocol::br_reinforcement(ostringStream, rsReinforcement);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENT_FAILED));
    }
    return QString::fromStdString(ostringStream.str());
}

void BRDBus::SetReinforcement(const QString& reinforcementXML)
{
    KLOG_DEBUG() << "Set reinforcement parameters, reinforcementXML : " << reinforcementXML;

    try
    {
        std::istringstream istringStream(reinforcementXML.toStdString());
        auto rsReinforcement = Protocol::br_reinforcement(istringStream, xml_schema::Flags::dont_validate);
        if (!this->m_configuration->setCustomRa(*rsReinforcement.get()))
        {
            sendErrorReply(QDBusError::InternalError,
                           BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_REINFORCEMENT_FAILED));
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_REINFORCEMENT_FAILED));
    }
}

void BRDBus::ResetReinforcement(const QString& name)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);

    this->m_configuration->delCustomRa(name);
    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Reset reinforcement parameters. name is %1").arg(name),
                    calledUniqueName);
}

void BRDBus::Scan(const QStringList& names)
{
    KLOG_DEBUG() << "Carry out scan progress. range is " << names.join(" ").toLocal8Bit();
    m_scanUniqueName = DBusHelper::getCallerUniqueName(this);

    // 已经在扫描则返回错误
    if (this->m_scanJob && this->m_scanJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SCAN_IS_RUNNING));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to scan."),
                      m_scanUniqueName);
        return;
    }
    m_jobResult.clear();

    try
    {
        this->m_scanJob = Job::create();
        for (auto iter = names.begin(); iter != names.end(); ++iter)
        {
            auto& name = (*iter);
            auto reinforcement = this->m_plugins->getReinforcement(name);

            if (!reinforcement)
            {
                sendErrorReply(QDBusError::InternalError,
                               BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND));
                SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                              tr("Failed to scan."),
                              m_scanUniqueName);
                return;
            }

            auto reinforcement_interface = this->m_plugins->getReinforcementInterface(reinforcement->getPluginName(),
                                                                                      reinforcement->getName());
            if (!reinforcement_interface)
            {
                sendErrorReply(QDBusError::InternalError,
                               BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND));
                SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                              tr("Failed to scan."),
                              m_scanUniqueName);
                return;
            }

            this->m_scanJob->addOperation(reinforcement->getPluginName(),
                                          reinforcement->getName(),
                                          [reinforcement_interface]() -> QString
                                          {
                                              QJsonObject retval;
                                              QString args;
                                              QString error;
                                              if (reinforcement_interface->get(args, error))
                                              {
                                                  retval[JOB_RETURN_VALUE] = StrUtils::str2jsonObject(args);
                                              }
                                              else
                                              {
                                                  retval[JOB_ERROR_STR] = error;
                                              }
                                              return StrUtils::json2str(retval);
                                          });
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SCAN_RANGE_INVALID));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to scan."),
                      m_scanUniqueName);
        return;
    }
    QObject::disconnect(this->m_scanJob.get(), &Job::processFinished, 0, 0);
    QObject::connect(this->m_scanJob.get(), &Job::processFinished, this, &BRDBus::finishedScanProgress);
    QObject::disconnect(this->m_scanJob.get(), &Job::processChanged, 0, 0);
    QObject::connect(this->m_scanJob.get(), &Job::processChanged, this, &BRDBus::scanResultHandle);

    if (!this->m_scanJob->runAsync())
    {
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SCAN_ALL_JOB_FAILED));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to scan."),
                      m_scanUniqueName);
        return;
    }
}

void BRDBus::reinforce(const QDBusMessage& message, const QStringList& names)
{
    SCOPE_EXIT(
        {
            QDBusConnection::systemBus().send(message.createReply());
        });
    KLOG_DEBUG() << "Carry out reinforcement progress. range is " << names.join(" ").toLocal8Bit();
    m_reforceUniqueName = message.service();
    m_isScanFlag = false;
    // 已经在加固则返回错误
    if (this->m_reinforceJob && this->m_reinforceJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        auto replyMessage = message.createErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCE_IS_RUNNING));
        QDBusConnection::systemBus().send(replyMessage);
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to reinforcement."),
                      m_reforceUniqueName);
        return;
    }
    this->m_reinforceJob = Job::create();
    // 加固前进行一次扫描
    Scan(names);
    m_jobResult.clear();

    for (auto iter = names.begin(); iter != names.end(); ++iter)
    {
        auto& name = (*iter);
        auto reinforcement = this->m_plugins->getReinforcement(name);
        if (!reinforcement)
        {
            auto replyMessage = message.createErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND));
            QDBusConnection::systemBus().send(replyMessage);
            SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                          tr("Failed to reinforcement."),
                          m_reforceUniqueName);
            return;
        }

        auto reinforcement_interface = this->m_plugins->getReinforcementInterface(reinforcement->getPluginName(),
                                                                                  reinforcement->getName());
        if (!reinforcement_interface)
        {
            auto replyMessage = message.createErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND));
            QDBusConnection::systemBus().send(replyMessage);
            SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                          tr("Failed to reinforcement."),
                          m_reforceUniqueName);
            return;
        }

        auto paramStr = getJsonParam(name);
        this->m_reinforceJob->addOperation(reinforcement->getPluginName(),
                                           reinforcement->getName(),
                                           [reinforcement_interface, paramStr]() -> QString
                                           {
                                               QString error;
                                               QJsonObject retval;
                                               if (!reinforcement_interface->set(paramStr, error))
                                               {
                                                   retval[JOB_ERROR_STR] = error;
                                               }
                                               else
                                               {
                                                   // 设置为空字符串，这里主要是为了区分加固成功和取消加固两种状态，后续可能会调整改逻辑
                                                   retval[JOB_RETURN_VALUE] = QString();
                                               }
                                               return StrUtils::json2str(retval);
                                           });
    }
    QObject::disconnect(this->m_reinforceJob.get(), &Job::processChanged, 0, 0);
    QObject::connect(this->m_reinforceJob.get(), &Job::processChanged, this, &BRDBus::reinforceResultHandle);
    QObject::disconnect(this->m_reinforceJob.get(), &Job::processFinished, 0, 0);
    QObject::connect(this->m_reinforceJob.get(), &Job::processFinished, this, &BRDBus::finishedReinforceProgress);

    if (!this->m_reinforceJob->runAsync())
    {
        auto replyMessage = message.createErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_CORE_REINFORCE_JOB_FAILED));
        QDBusConnection::systemBus().send(replyMessage);
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to reinforcement."),
                      m_reforceUniqueName);
        return;
    }
}

void BRDBus::Cancel(const qlonglong& jobID)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    BRErrorCode errorCode = BRErrorCode::SUCCESS;

    if (this->m_scanJob &&
        jobID == this->m_scanJob->getId() &&
        this->m_scanJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        if (!this->m_scanJob->cancel())
        {
            errorCode = BRErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_1;
        }
    }
    else if (this->m_reinforceJob &&
             jobID == this->m_reinforceJob->getId() &&
             this->m_reinforceJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        if (!this->m_reinforceJob->cancel())
        {
            errorCode = BRErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_2;
        }
    }
    else
    {
        errorCode = BRErrorCode::ERROR_DAEMON_CANCEL_NOTFOUND_JOB;
    }

    if (errorCode != BRErrorCode::SUCCESS)
    {
        sendErrorReply(QDBusError::Failed, BR_ERROR2STR(errorCode));
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to cancel progress."),
                      calledUniqueName);
        return;
    }

    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Cancel. job id: %1").arg(jobID),
                    calledUniqueName);
}

void BRDBus::ExportStrategy(bool operationResult)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    SSR_LOG(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("export strategy"),
                    operationResult,
                    calledUniqueName);
}
void BRDBus::GenerateReport(bool operationResult)
{
    auto calledUniqueName = DBusHelper::getCallerUniqueName(this);
    SSR_LOG(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("export report"),
                    operationResult,
                    calledUniqueName);
}

void BRDBus::setFallback(const QDBusMessage& message, const uint32_t& snapshotStatus)
{
    SCOPE_EXIT(
        {
            QDBusConnection::systemBus().send(message.createReply());
        });
    auto calledUniqueName = message.service();

    KLOG_INFO("Set fallback. snapshotStatus: %d.", snapshotStatus);
    RETURN_IF_TRUE(snapshotStatus == BRFallbackMethod::BR_FALLBACK_METHOD_OTHER);
    QStringList names_rh;
    auto reinforcements = this->m_plugins->getReinforcements();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        auto& rsReinforcement = (*iter)->getRs();
        names_rh.push_back(QString::fromStdString(rsReinforcement.name()));
    }
    if (names_rh.empty())
    {
        // 需回退的加固项为空 不需要进行加固了 reinforce Finish
        emit ProgressFinished();
        this->m_configuration->setFallbackStatus(BR_FALLBACK_STATUS_IS_FINISHED);
        auto replyMessage = message.createReply();
        QDBusConnection::systemBus().send(replyMessage);
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set fallback."),
                      calledUniqueName);
        return;
    }

    // 已经在加固则返回错误
    if (this->m_reinforceJob && this->m_reinforceJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        auto replyMessage = message.createErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCE_IS_RUNNING));
        QDBusConnection::systemBus().send(replyMessage);
        this->m_configuration->setFallbackStatus(BR_FALLBACK_STATUS_IS_FINISHED);
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                      tr("Failed to set fallback."),
                      calledUniqueName);
        return;
    }

    m_fallbackMethod = BRFallbackMethod(snapshotStatus);
    reinforce(message, names_rh);
    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Set fallback. snapshot status is %1").arg(snapshotStatus == BR_FALLBACK_METHOD_INITIAL ? tr("init") : tr("pre")),
                    calledUniqueName);
    m_fallbackMethod = BRFallbackMethod::BR_FALLBACK_METHOD_OTHER;
}

void BRDBus::writeScanResultLog()
{
    int successCount = 0;
    int failCount = 0;
    // 扫描完成时，根据结果写日志
    for (auto name : m_jobResult.keys())
    {
        // 成功
        if (m_jobResult.value(name) == static_cast<int32_t>(BR_REINFORCEMENT_STATE_SAFE | BR_REINFORCEMENT_STATE_SCAN_DONE))
        {
            successCount++;
            continue;
        }
        if (m_jobResult.value(name) == static_cast<int32_t>(BR_REINFORCEMENT_STATE_SAFE | BR_REINFORCEMENT_STATE_SCAN_ERROR))
        {
            failCount++;
            continue;
        }
    }
    SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                    tr("Scan success %1, safe %2, unsafe %3.")
                        .arg(QString::number(successCount + failCount), QString::number(successCount), QString::number(failCount)),
                    m_scanUniqueName);
}

void BRDBus::writeReinforcementResultLog()
{
    int successCount = 0;
    int failCount = 0;
    // 加固完成时，根据结果写日志
    for (auto name : m_jobResult.keys())
    {
        // 成功
        if (static_cast<BRReinforcementState>(m_jobResult.value(name)) == BR_REINFORCEMENT_STATE_REINFORCE_DONE)
        {
            successCount++;
            continue;
        }
        if (static_cast<BRReinforcementState>(m_jobResult.value(name)) == BR_REINFORCEMENT_STATE_REINFORCE_ERROR)
        {
            failCount++;
            continue;
        }
    }
    if (successCount == 0)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                        tr("Reinforcement fail %1")
                            .arg(failCount),
                        m_reforceUniqueName);
    }
    else if (failCount != 0 && successCount != 0)
    {
        SSR_LOG_ERROR(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                        tr("Reinforcement success %1, fail %2")
                            .arg(successCount, failCount),
                        m_reforceUniqueName);
    }
    else
    {
        SSR_LOG_SUCCESS(Log::Manager::LogType::BASELINE_REINFORCEMENT,
                        tr("Reinforcement success %1")
                            .arg(successCount),
                        m_reforceUniqueName);
    }
}

void BRDBus::init()
{
    QDBusConnection dbusConnection = QDBusConnection::systemBus();
    if (!dbusConnection.registerObject(BR_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "register Service error:" << dbusConnection.lastError().message();
        return;
    }

    this->m_configuration = Configuration::getInstance();
    this->m_categories = Categories::getInstance();
    this->m_plugins = Plugins::getInstance();

    this->m_resourceMonitor = new ResourceMonitor();
    KLOG_DEBUG("init ResourceMonitor.");
    QObject::connect(this->m_resourceMonitor, &ResourceMonitor::homeFreeSpaceRatio_,
                     this, &BRDBus::homeFreeSpaceRatio);
    QObject::connect(this->m_resourceMonitor, &ResourceMonitor::rootFreeSpaceRatio_,
                     this, &BRDBus::rootFreeSpaceRatio);
    QObject::connect(this->m_resourceMonitor, &ResourceMonitor::cpuAverageLoadRatio_,
                     this, &BRDBus::cpuAverageLoadRatio);
    QObject::connect(this->m_resourceMonitor, &ResourceMonitor::memoryRemainingRatio_, this, &BRDBus::memoryRemainingRatio);

    // 进程完成后，回退状态置为未开始
    QObject::connect(this, &BRDBus::ProgressFinished, this, [this]()
                     {
                         RETURN_IF_TRUE(BR_FALLBACK_STATUS_NOT_STARTED == this->m_configuration->getFallbackStatus());
                         if (!this->m_configuration->setFallbackStatus(BR_FALLBACK_STATUS_NOT_STARTED))
                         {
                             KLOG_ERROR() << "set fallback status failed.";
                         }
                     });

    if (m_configuration->getResourceMonitorStatus() == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN)
    {
        m_resourceMonitorTimer = new QTimer(this);
        QObject::connect(this->m_resourceMonitorTimer, &QTimer::timeout, this, &BRDBus::setResourceMonitor);
        m_resourceMonitorTimer->start(RESOURCEMONITORMS);
    }
    // 读取加固项状态
    connect(this, &BRDBus::ReinforceProgress, this, &BRDBus::readReinforceItemStatus);
    connect(this, &BRDBus::ScanProgress, this, &BRDBus::readReinforceItemStatus);

    // 服务启动时自动扫描一次，获取系统默认配置存入rh-first文件
    if (!QFile::exists(RH_BR_OPERATE_DATA_FIRST))
    {
        QStringList names;
        auto reinforcements = this->m_plugins->getReinforcements();
        for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
        {
            auto& rsReinforcement = (*iter)->getRs();
            names.push_back(QString::fromStdString(rsReinforcement.name()));
        }
        m_isScanFlag = false;
        m_isFinishRHWrite = false;
        Scan(names);
    }
}

void BRDBus::scanResultHandle(const JobResult& jobResult)
{
    Protocol::JobResult scanResult(0, 0, 0);
    try
    {
        scanResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
        scanResult.job_id(jobResult.job_id);
        scanResult.job_state(this->m_scanJob->getState());

        for (auto iter = jobResult.running_operations.begin(); iter != jobResult.running_operations.end(); ++iter)
        {
            auto operation = this->m_scanJob->getOperation((*iter));

            Protocol::ReinforcementResult reinforcementResult(std::string(), 0);
            reinforcementResult.name(operation->reinforcement_name.toStdString());
            reinforcementResult.state(BRReinforcementState::BR_REINFORCEMENT_STATE_SCANNING);
            reinforcementResult.args("");
            scanResult.reinforcement().push_back(std::move(reinforcementResult));
        }

        for (auto iter = jobResult.current_finished_operations.begin(); iter != jobResult.current_finished_operations.end(); ++iter)
        {
            auto& operationResult = (*iter);
            auto operation = this->m_scanJob->getOperation(operationResult.operation_id);
            Protocol::ReinforcementResult reinforcementResult(std::string(), 0);

            reinforcementResult.name(operation->reinforcement_name.toStdString());

            BRReinforcementState state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNKNOWN;
            const auto resultValues = StrUtils::str2jsonObject(operationResult.result);
            // 如果结果为空应该时任务被取消了，如果在收到客户端的任务取消命令时操作已经在执行，结果也可能不为空，所以这里不能通过任务是否被取消的状态来判断
            if (resultValues.isEmpty())
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNSCAN;
                reinforcementResult.args("");
            }
            else if (resultValues[JOB_ERROR_STR].isString())
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_SCAN_ERROR;
                reinforcementResult.args("");
                reinforcementResult.error(resultValues[JOB_ERROR_STR].toString().toStdString());
            }
            else
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_SCAN_DONE;
                reinforcementResult.args(StrUtils::json2str(resultValues).toStdString());
            }
            auto reinforcement = this->m_plugins->getReinforcement(operation->reinforcement_name);
            updateRH(operation->reinforcement_name, resultValues[JOB_RETURN_VALUE].toObject());

            if ((state & BRReinforcementState::BR_REINFORCEMENT_STATE_SCAN_DONE) != 0 &&
                reinforcement &&
                reinforcement->matchRules(resultValues[JOB_RETURN_VALUE].toObject()))
            {
                state = BRReinforcementState(state | BRReinforcementState::BR_REINFORCEMENT_STATE_SAFE);
            }
            else
            {
                // 保留未扫描状态，否则扫描结果仅为符合和不符合了
                if (state != BRReinforcementState::BR_REINFORCEMENT_STATE_UNSCAN)
                {
                    state = BRReinforcementState(state | BRReinforcementState::BR_REINFORCEMENT_STATE_UNSAFE);
                }
            }
            reinforcementResult.state(int32_t(state));
            scanResult.reinforcement().push_back(std::move(reinforcementResult));
        }

        if (m_isScanFlag)
        {
            std::ostringstream ostringStream;
            Protocol::br_job_result(ostringStream, scanResult);
            emit ScanProgress(QString(ostringStream.str().c_str()));
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }
}

void BRDBus::reinforceResultHandle(const JobResult& jobResult)
{
    Protocol::JobResult reinforceResult(0, 0, 0);
    try
    {
        reinforceResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
        reinforceResult.job_id(jobResult.job_id);
        reinforceResult.job_state(this->m_reinforceJob->getState());

        for (auto iter = jobResult.running_operations.begin(); iter != jobResult.running_operations.end(); ++iter)
        {
            auto operation = this->m_reinforceJob->getOperation(*iter);
            Protocol::ReinforcementResult reinforcementResult(std::string(), 0);

            reinforcementResult.name(operation->reinforcement_name.toStdString());
            reinforcementResult.state(BRReinforcementState::BR_REINFORCEMENT_STATE_REINFORCING);
            reinforceResult.reinforcement().push_back(std::move(reinforcementResult));
        }

        for (auto iter = jobResult.current_finished_operations.begin(); iter != jobResult.current_finished_operations.end(); ++iter)
        {
            auto& operationResult = (*iter);
            auto operation = this->m_reinforceJob->getOperation(operationResult.operation_id);
            Protocol::ReinforcementResult reinforcementResult(std::string(), 0);

            reinforcementResult.name(operation->reinforcement_name.toStdString());

            BRReinforcementState state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNKNOWN;
            auto resultValues = StrUtils::str2jsonObject(operationResult.result);
            if (resultValues.isEmpty())
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNREINFORCE;
            }
            else if (resultValues[JOB_ERROR_STR].isString())
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_REINFORCE_ERROR;
                reinforcementResult.error(resultValues[JOB_ERROR_STR].toString().toStdString());
            }
            else
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_REINFORCE_DONE;
            }
            reinforcementResult.state(int32_t(state));
            reinforceResult.reinforcement().push_back(std::move(reinforcementResult));
        }
        // 回退中，不关注进程信息
        if (BR_FALLBACK_STATUS_IN_PROGRESS != this->m_configuration->getFallbackStatus())
        {
            std::ostringstream ostringStream;
            Protocol::br_job_result(ostringStream, reinforceResult);
            emit ReinforceProgress(QString(ostringStream.str().c_str()));
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }
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

void BRDBus::finishedScanProgress()
{
    // 根据结果写日志，处于回退中和加固前的扫描不需要写
    if (BR_FALLBACK_STATUS_IN_PROGRESS != this->m_configuration->getFallbackStatus() && m_isScanFlag)
    {
        writeScanResultLog();
    }
    m_isScanFlag = true;
    m_isFinishRHWrite = true;
    // 回退中，不关注扫描完成
    if (BR_FALLBACK_STATUS_IN_PROGRESS != this->m_configuration->getFallbackStatus())
    {
        emit ProgressFinished();
    }
}

void BRDBus::finishedReinforceProgress()
{
    // 根据结果写日志，回退不需要写
    if (BR_FALLBACK_STATUS_IN_PROGRESS != this->m_configuration->getFallbackStatus())
    {
        writeReinforcementResultLog();
    }
    this->m_configuration->setFallbackStatus(BR_FALLBACK_STATUS_IS_FINISHED);
    m_isScanFlag = true;
    emit ProgressFinished();
}

void BRDBus::parseJsonParam(const Protocol::Reinforcement::ArgSequence& argSequence, QJsonObject& param)
{
    for (auto argIter = argSequence.begin(); argIter != argSequence.end(); ++argIter)
    {
        QString inputExample = argIter->input_example() != nullptr ? argIter->input_example().get().c_str() : "";

        // str2jsonValue中的类型转换没法区分line输入纯数字和数字输入框spin输入的纯数字，都会被转为double类型，这里需要进行判断,
        // 如果存在inputExample则肯定为line输入的纯数字，参数应该为str类型
        param.insert(argIter->name().c_str(), inputExample.isEmpty() ? StrUtils::str2jsonValue(argIter->value())
                                                                     : QJsonValue::fromVariant(argIter->value().c_str()));
    }
}

QString BRDBus::getJsonParam(const QString& reinforceName)
{
    QJsonObject param;
    QString paramStr;
    auto reinforcement = this->m_plugins->getReinforcement(reinforceName);
    if (m_fallbackMethod == BR_FALLBACK_METHOD_OTHER)
    {
        auto& args = reinforcement->getRs().arg();
        parseJsonParam(args, param);
        paramStr = StrUtils::json2str(param);
    }
    else
    {
        // 获取rh文件里面的参数
        auto reinforcements = this->m_plugins->getReinforcements();
        for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
        {
            auto& rsReinforcement = (*iter)->getRs();
            CONTINUE_IF_TRUE(rsReinforcement.name() != reinforceName.toStdString());
            auto& iter_args = rsReinforcement.arg();
            parseJsonParam(iter_args, param);
            paramStr = StrUtils::json2str(param);
        }
        auto rhLast = this->m_configuration->readRhFromFile(m_fallbackMethod == BR_FALLBACK_METHOD_LAST ? RH_BR_OPERATE_DATA_LAST : RH_BR_OPERATE_DATA_FIRST);
        auto& rhLastReinforcements = rhLast->reinforcement();

        for (auto iter = rhLastReinforcements.begin(); iter != rhLastReinforcements.end(); ++iter)
        {
            CONTINUE_IF_TRUE(iter->name() != reinforceName.toStdString());
            auto& iter_args = iter->arg();
            parseJsonParam(iter_args, param);
        }

        paramStr = StrUtils::json2str(param);
    }
    return paramStr;
}

void BRDBus::updateRH(const QString& reinforceName, const QJsonObject& resultReturnValue)
{
    auto reinforcement = this->m_plugins->getReinforcement(reinforceName);

    // 上一次历史操作存入rh文件
    if (!m_isScanFlag && BR_FALLBACK_STATUS_IN_PROGRESS != this->m_configuration->getFallbackStatus())
    {
        auto rhReinforcement = reinforcement->getRs();
        auto& iterArgs = rhReinforcement.arg();
        for (auto iterArg = iterArgs.begin(); iterArg != iterArgs.end(); ++iterArg)
        {
            CONTINUE_IF_TRUE(resultReturnValue[iterArg->name().c_str()].toVariant().toString().isEmpty());
            iterArg->value(resultReturnValue[iterArg->name().c_str()].toVariant().toString().toStdString());
        }
        this->m_configuration->setCustomRh(rhReinforcement, RH_BR_OPERATE_DATA_LAST);
        if (!m_isFinishRHWrite)
        {
            // 首次加固修改保存的历史操作文件
            this->m_configuration->setCustomRh(rhReinforcement, RH_BR_OPERATE_DATA_FIRST);
        }
    }
}

void BRDBus::readReinforceItemStatus(const QString &jobResult)
{
    RETURN_IF_TRUE(jobResult.isEmpty())

    std::istringstream istringStream(jobResult.toStdString());
    auto result = KS::Protocol::br_job_result(istringStream, xml_schema::Flags::dont_validate);
    for (auto reinforcement : result->reinforcement())
    {
        m_jobResult.insert(QString::fromStdString(reinforcement.name()), static_cast<int32_t>(reinforcement.state()));
    }
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
    this->MemoryAbnormal(QString(std::to_string(memoryRatio).c_str()));
}

}  // namespace BRDaemon
}  // namespace KS
