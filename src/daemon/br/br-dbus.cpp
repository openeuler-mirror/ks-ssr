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
#include "br-protocol.hxx"
#include "categories.h"
#include "configuration.h"
#include "include/ssr-marcos.h"
#include "plugins.h"
#include "src/daemon/br_adaptor.h"
#include "src/daemon/common/polkit-proxy.h"
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
    : QObject(parent), timer(nullptr)
{
    this->m_dbus = new BRAdaptor(this);
    init();
}

BRDBus::~BRDBus()
{
    if (this->timer)
    {
        timer->stop();
        QObject::disconnect(this->timer, SIGNAL(QTimer::timeout()), this, SLOT(BRDBus::onResourceMonitor()));
        delete this->timer;
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
    KLOG_INFO("SetStandardType. standard type: %d.", standardType);

    if (standardType >= BRStandardType::BR_STANDARD_TYPE_LAST)
    {
        sendErrorReply(QDBusError::InvalidArgs, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_STANDARD_TYPE_INVALID));
    }

    RETURN_IF_TRUE(standardType == this->m_configuration->getStandardType())

    if (!this->m_configuration->setStandardType(BRStandardType(standardType)))
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_STANDARD_TYPE_FAILED));
    }
}

void BRDBus::ImportCustomRS(const QString& encodedStandard)
{
    KLOG_INFO() << "Import custom reinforce standard. encoded standard: " << encodedStandard;

    BRErrorCode error_code = BRErrorCode::SUCCESS;
    if (!this->m_configuration->setCustomRs(encodedStandard, error_code))
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(error_code));
    }
}

void BRDBus::SetStrategyType(const uint32_t& strategyType)
{
    KLOG_INFO("SetStrategyType. strategy type: %d.", strategyType);

    if (strategyType >= BRStrategyType::BR_STRATEGY_TYPE_LAST)
    {
        sendErrorReply(QDBusError::InvalidArgs, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_STRATEGY_TYPE_INVALID));
    }
    RETURN_IF_TRUE(strategyType == this->m_configuration->getStrategyType())

    if (!this->m_configuration->setStrategyType(BRStrategyType(strategyType)))
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_STRATEGY_TYPE_FAILED));
    }
}

void BRDBus::SetTimeScan(const uint32_t& timeScan)
{
    KLOG_INFO("Set time scan: %d.", timeScan);

    RETURN_IF_TRUE(timeScan == uint32_t(this->m_configuration->getTimeScan()))

    if (!this->m_configuration->setTimeScan(int(timeScan)))
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_TIME_SCAN_FAILED));
    }
}

void BRDBus::SetNotificationStatus(const uint32_t& notificationStatus)
{
    KLOG_INFO("Set notification status: %d.", notificationStatus);

    if (notificationStatus >= BRNotificationStatus::BR_NOTIFICATION_STATUS_OTHER)
    {
        sendErrorReply(QDBusError::InvalidArgs,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_NOTIFICATION_STATUS_INVALID));
    }
    RETURN_IF_TRUE(notificationStatus == this->m_configuration->getNotificationStatus())

    if (!this->m_configuration->setNotificationStatus(BRNotificationStatus(notificationStatus)))
    {
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_NOTIFICATION_STATUS_FAILED));
    }
}

void BRDBus::SetFallbackStatus(const uint32_t& fallbackStatus)
{
    KLOG_INFO("Set fallback status: %d.", fallbackStatus);

    if (fallbackStatus > BRFallbackStatus::BR_FALLBACK_STATUS_IS_FINISHED)
    {
        sendErrorReply(QDBusError::InvalidArgs,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_FALLBACK_STATUS_INVALID));
    }
    RETURN_IF_TRUE(fallbackStatus == this->m_configuration->getFallbackStatus())

    if (!this->m_configuration->setFallbackStatus(BRFallbackStatus(fallbackStatus)))
    {
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_FALLBACK_STATUS_FAILED));
    }
}

void BRDBus::ImportCustomRA(const QString& encodedStrategy)
{
    KLOG_DEBUG() << "Import custom reinforce strategy. encoded strategy: " << encodedStrategy;
    try
    {
        std::ofstream ofs(CUSTOM_RA_STRATEGY_FILEPATH, std::ios_base::out);
        ofs << encodedStrategy.toStdString();
        ofs.close();
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        return;
    }
    if (!m_configuration->checkRaStrategy())
    {
        remove(CUSTOM_RA_STRATEGY_FILEPATH);
        // 不知道选选哪个错误码，所以选择了 ERROR_FAILED
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_FAILED));
    }
}

void BRDBus::SetCheckBox(const QString& reinforcementName, const bool& checkboxStatus)
{
    KLOG_DEBUG() << "reinforcementName: " << reinforcementName << ", SetCheckBox";
    m_configuration->setRaCheckbox(reinforcementName, checkboxStatus);
}

void BRDBus::SetResourceMonitorSwitch(const uint32_t& resourceMonitor)
{
    KLOG_INFO("SetResourceMonitorSwitch. resource monitor: %d.", resourceMonitor);

    if (resourceMonitor >= BRResourceMonitor::BR_RESOURCE_MONITOR_OTHER)
    {
        sendErrorReply(QDBusError::InvalidArgs, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_RESOURCE_MONITOR_INVALID));
    }

    RETURN_IF_TRUE(resourceMonitor == this->m_configuration->getResourceMonitorStatus())

    if (this->m_configuration->setResourceMonitorStatus(BRResourceMonitor(resourceMonitor)))
    {
        timer->stop();
        QObject::disconnect(this->timer, SIGNAL(timeout()), this, SLOT(BRDBus::onResourceMonitor()));
        if (BRResourceMonitor(resourceMonitor) == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN)
        {
            this->timer = new QTimer();
            QObject::connect(this->timer, &QTimer::timeout, this, &BRDBus::onResourceMonitor);
            this->timer->start(RESOURCEMONITORMS);
        }
    }
    else
    {
        sendErrorReply(QDBusError::InvalidArgs, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_RESOURCE_MONITOR_FAILED));
    }
}

QString BRDBus::GetCategories()
{
    KLOG_DEBUG("GetCategories");

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
    std::ostringstream ostring_stream;
    auto rs = this->m_configuration->getRs();

    if (!rs)
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_GET_RS_FAILED));
    }

    try
    {
        Protocol::br_rs(ostring_stream, *rs.get());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_GET_RS_FAILED));
    }
    return QString::fromStdString(ostring_stream.str());
}

QString BRDBus::GetReinforcements()
{
    KLOG_DEBUG() << "GetReinforcements";

    std::ostringstream ostring_stream;
    Protocol::Reinforcements protocol_reinforcements;

    auto reinforcements = this->m_plugins->getReinforcements();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        auto& rs_reinforcement = (*iter)->getRs();
        protocol_reinforcements.reinforcement().push_back(rs_reinforcement);
    }

    try
    {
        Protocol::br_reinforcements(ostring_stream, protocol_reinforcements);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENT_FAILED));
    }
    return QString::fromStdString(ostring_stream.str());
}

void BRDBus::ResetReinforcements()
{
    KLOG_INFO("Reset all reinforcement parameters.");

    this->m_configuration->delAllCustomRa();
}

QString BRDBus::GetReinforcement(const QString& name)
{
    KLOG_DEBUG() << "GetReinforcement";

    std::ostringstream ostring_stream;
    auto reinforcement = this->m_plugins->getReinforcement(name);
    if (!reinforcement)
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND));
    }
    auto& rs_reinforcement = reinforcement->getRs();

    try
    {
        Protocol::br_reinforcement(ostring_stream, rs_reinforcement);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENT_FAILED));
    }
    return QString::fromStdString(ostring_stream.str());
}

void BRDBus::SetReinforcement(const QString& reinforcementXML)
{
    KLOG_DEBUG() << "Set reinforcement parameters, reinforcementXML : " << reinforcementXML;

    try
    {
        std::istringstream istring_stream(reinforcementXML.toStdString());
        auto rs_reinforcement = Protocol::br_reinforcement(istring_stream, xml_schema::Flags::dont_validate);
        if (!this->m_configuration->setCustomRa(*rs_reinforcement.get()))
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
    KLOG_DEBUG() << "Reset reinforcement parameters. name = " << name;
    this->m_configuration->delCustomRa(name);
}

void BRDBus::Scan(const QStringList& names)
{
    KLOG_INFO() << "Scan. range : " << names.join(" ").toLocal8Bit();

    // 已经在扫描则返回错误
    if (this->m_scanJob && this->m_scanJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SCAN_IS_RUNNING));
        // THROW_DBUSCXX_ERROR(BRErrorCode::ERROR_DAEMON_SCAN_IS_RUNNING);
    }

    try
    {
        this->m_scanJob = Job::create();
        if (!m_isFristReinfoce)
            m_isFristReinfoceFinish = false;
        for (auto iter = names.begin(); iter != names.end(); ++iter)
        {
            auto& name = (*iter);
            auto reinforcement = this->m_plugins->getReinforcement(name);

            if (!reinforcement)
            {
                sendErrorReply(QDBusError::InternalError,
                               BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND));
            }

            auto reinforcement_interface = this->m_plugins->getReinforcementInterface(reinforcement->getPluginName(),
                                                                                      reinforcement->getName());
            if (!reinforcement_interface)
            {
                sendErrorReply(QDBusError::InternalError,
                               BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND));
            }

            this->m_scanJob->addOperation(reinforcement->getPluginName(),
                                          reinforcement->getName(),
                                          [reinforcement_interface]() -> QString
                                          {
                                              //    QJsonValue retval;
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
    }
    QObject::disconnect(this->m_scanJob.get(), &Job::process_finished_, 0, 0);
    QObject::connect(this->m_scanJob.get(), &Job::process_finished_, this, &BRDBus::scanProgressFinished);
    QObject::disconnect(this->m_scanJob.get(), &Job::process_changed_, 0, 0);
    QObject::connect(this->m_scanJob.get(), &Job::process_changed_, this, &BRDBus::onScanProcessChangedCb);

    if (!this->m_scanJob->runAsync())
    {
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SCAN_ALL_JOB_FAILED));
    }
}

void BRDBus::reinforce(const QDBusMessage& message, const QStringList& names)
{
    SCOPE_EXIT({
        QDBusConnection::systemBus().send(message.createReply());
    });
    KLOG_INFO() << "Reinforce. range : " << names.join(" ").toLocal8Bit();
    m_isScanFlag = false;
    // 已经在加固则返回错误
    if (this->m_reinforceJob && this->m_reinforceJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        auto replyMessage = message.createErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCE_IS_RUNNING));
        QDBusConnection::systemBus().send(replyMessage);
        return;
    }

    this->m_reinforceJob = Job::create();

    // 首次加固保存所有加固前配置
    m_isFristReinfoceFinish = false;

    if (m_isFristReinfoce)
    {
        QStringList names_rh;
        auto rh = this->m_configuration->readRhFromFile(RH_BR_OPERATE_DATA_FIRST);
        if (rh->reinforcement().empty())
        {
            auto reinforcements = this->m_plugins->getReinforcements();
            for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
            {
                auto& rs_reinforcement = (*iter)->getRs();
                names_rh.push_back(QString::fromStdString(rs_reinforcement.name()));
                rh->reinforcement().push_back(rs_reinforcement);
            }
            this->m_configuration->writeRhToFile(rh, RH_BR_OPERATE_DATA_FIRST);

            m_isFristReinfoceFinish = true;
            Scan(names_rh);
        }
        // 此处需等待扫描进程完成后置为false
        m_isFristReinfoce = false;
    }
    else
    {
        KLOG_DEBUG("Reinforce m_isScanFlag = %d", m_isScanFlag);
        Scan(names);
    }

    for (auto iter = names.begin(); iter != names.end(); ++iter)
    {
        auto& name = (*iter);
        auto reinforcement = this->m_plugins->getReinforcement(name);
        if (!reinforcement)
        {
            auto replyMessage = message.createErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND));
            QDBusConnection::systemBus().send(replyMessage);
            return;
        }

        auto reinforcement_interface = this->m_plugins->getReinforcementInterface(reinforcement->getPluginName(),
                                                                                  reinforcement->getName());
        if (!reinforcement_interface)
        {
            auto replyMessage = message.createErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND));
            QDBusConnection::systemBus().send(replyMessage);
            return;
        }

        QJsonObject param;
        QString param_str = "";

        if (m_fallbackMethod == BRFallbackMethod::BR_FALLBACK_METHOD_INITIAL)
        {
            auto rh = this->m_configuration->readRhFromFile(RH_BR_OPERATE_DATA_FIRST);
            auto& rh_reinforcements = rh->reinforcement();
            for (auto iter = rh_reinforcements.begin(); iter != rh_reinforcements.end(); ++iter)
            {
                CONTINUE_IF_TRUE(iter->name() != name.toStdString());
                auto& iter_args = iter->arg();
                for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                {
                    param.insert(iter_arg->name().c_str(), StrUtils::str2jsonValue(iter_arg->value()));
                }
                param_str = StrUtils::json2str(param);
            }
            KLOG_DEBUG() << "frist fallback name : " << name << ", frist fallback param_str: " << param_str;
        }
        else if (m_fallbackMethod == BRFallbackMethod::BR_FALLBACK_METHOD_LAST)
        {
            //
            auto rh = this->m_configuration->readRhFromFile(RH_BR_OPERATE_DATA_FIRST);
            auto& rh_reinforcements = rh->reinforcement();
            for (auto iter = rh_reinforcements.begin(); iter != rh_reinforcements.end(); ++iter)
            {
                CONTINUE_IF_TRUE(iter->name() != name.toStdString());
                auto& iter_args = iter->arg();
                for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                {
                    param.insert(iter_arg->name().c_str(), StrUtils::str2jsonValue(iter_arg->value()));
                }
                param_str = StrUtils::json2str(param);
            }
            auto rh_last = this->m_configuration->readRhFromFile(RH_BR_OPERATE_DATA_LAST);
            auto& rh_last_reinforcements = rh_last->reinforcement();

            for (auto iter = rh_last_reinforcements.begin(); iter != rh_last_reinforcements.end(); ++iter)
            {
                CONTINUE_IF_TRUE(iter->name() != name.toStdString());
                auto& iter_args = iter->arg();
                for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                {
                    param.insert(iter_arg->name().c_str(), StrUtils::str2jsonValue(iter_arg->value()));
                }
            }

            param_str = StrUtils::json2str(param);
        }
        else
        {
            auto& args = reinforcement->getRs().arg();
            for (auto arg_iter = args.begin(); arg_iter != args.end(); ++arg_iter)
            {
                param.insert(arg_iter->name().c_str(), StrUtils::str2jsonValue(arg_iter->value()));
            }
            param_str = StrUtils::json2str(param);
        }

        this->m_reinforceJob->addOperation(reinforcement->getPluginName(),
                                           reinforcement->getName(),
                                           [reinforcement_interface, param_str]() -> QString
                                           {
                                               QString error;
                                               QJsonObject retval;
                                               if (!reinforcement_interface->set(param_str, error))
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
    QObject::disconnect(this->m_reinforceJob.get(), &Job::process_changed_, 0, 0);
    QObject::connect(this->m_reinforceJob.get(), &Job::process_changed_, this, &BRDBus::onReinforceProcessChangedCb);
    QObject::disconnect(this->m_reinforceJob.get(), &Job::process_finished_, 0, 0);
    QObject::connect(this->m_reinforceJob.get(), &Job::process_finished_, this, &BRDBus::reinforceProgressFinished);

    if (!this->m_reinforceJob->runAsync())
    {
        auto replyMessage = message.createErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_CORE_REINFORCE_JOB_FAILED));
        QDBusConnection::systemBus().send(replyMessage);
        return;
    }
}

void BRDBus::Cancel(const qlonglong& jobID)
{
    KLOG_INFO("Cancel. job id: %lld.", jobID);

    BRErrorCode error_code = BRErrorCode::SUCCESS;

    if (this->m_scanJob &&
        jobID == this->m_scanJob->getId() &&
        this->m_scanJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        if (!this->m_scanJob->cancel())
        {
            error_code = BRErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_1;
        }
    }
    else if (this->m_reinforceJob &&
             jobID == this->m_reinforceJob->getId() &&
             this->m_reinforceJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        if (!this->m_reinforceJob->cancel())
        {
            error_code = BRErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_2;
        }
    }
    else
    {
        error_code = BRErrorCode::ERROR_DAEMON_CANCEL_NOTFOUND_JOB;
    }

    if (error_code != BRErrorCode::SUCCESS)
    {
        sendErrorReply(QDBusError::Failed, BR_ERROR2STR(error_code));
    }
}

void BRDBus::setFallback(const QDBusMessage& message, const uint32_t& snapshotStatus)
{
    SCOPE_EXIT({
        QDBusConnection::systemBus().send(message.createReply());
    });
    KLOG_INFO("Set fallback. snapshotStatus: %d.", snapshotStatus);
    RETURN_IF_TRUE(snapshotStatus == BRFallbackMethod::BR_FALLBACK_METHOD_OTHER);
    QStringList names_rh;
    auto rh = this->m_configuration->readRhFromFile(RH_BR_OPERATE_DATA_FIRST);
    auto& rh_reinforcements = rh->reinforcement();
    if (rh_reinforcements.empty())
    {
        auto replyMessage = message.createErrorReply(QDBusError::Failed, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_FALLBACK_RH_EMPTY));
        QDBusConnection::systemBus().send(replyMessage);
        this->m_configuration->setFallbackStatus(BR_FALLBACK_STATUS_IS_FINISHED);
        return;
    }
    for (auto iter = rh_reinforcements.begin(); iter != rh_reinforcements.end(); ++iter)
    {
        // 获取所有加固项name
        names_rh.push_back(QString::fromStdString(iter->name()));
    }
    if (names_rh.empty())
    {
        // 需回退的加固项为空 不需要进行加固了 reinforce Finish
        emit ProgressFinished();
        this->m_configuration->setFallbackStatus(BR_FALLBACK_STATUS_IS_FINISHED);
        auto replyMessage = message.createReply();
        QDBusConnection::systemBus().send(replyMessage);
        return;
    }

    // 已经在加固则返回错误
    if (this->m_reinforceJob && this->m_reinforceJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        auto replyMessage = message.createErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCE_IS_RUNNING));
        QDBusConnection::systemBus().send(replyMessage);
        this->m_configuration->setFallbackStatus(BR_FALLBACK_STATUS_IS_FINISHED);
        return;
    }

    m_fallbackMethod = BRFallbackMethod(snapshotStatus);
    reinforce(message, names_rh);
    m_fallbackMethod = BRFallbackMethod::BR_FALLBACK_METHOD_OTHER;
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
        timer = new QTimer();
        QObject::connect(this->timer, &QTimer::timeout, this, &BRDBus::onResourceMonitor);
        timer->start(RESOURCEMONITORMS);
    }
}

void BRDBus::onScanProcessChangedCb(const JobResult& jobResult)
{
    KLOG_DEBUG() << "onScanProcessChangedCb";

    Protocol::JobResult scanResult(0, 0, 0);

    try
    {
        scanResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
        scanResult.job_id(jobResult.job_id);
        scanResult.job_state(this->m_scanJob->getState());

        int32_t itemCount = 0;
        for (auto iter = jobResult.running_operations.begin(); iter != jobResult.running_operations.end(); ++iter)
        {
            auto operation = this->m_scanJob->getOperation((*iter));

            Protocol::ReinforcementResult reinforcementResult(std::string(), 0);
            reinforcementResult.name(operation->reinforcement_name.toStdString());
            reinforcementResult.state(BRReinforcementState::BR_REINFORCEMENT_STATE_SCANNING);
            reinforcementResult.args("");
            scanResult.reinforcement().push_back(std::move(reinforcementResult));
            ++itemCount;
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

            // 上一次历史操作存入rh文件
            if (!m_isScanFlag)
            {
                auto rh_reinforcement = reinforcement->getRs();
                auto& iter_args = rh_reinforcement.arg();
                for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                {
                    if (!resultValues[JOB_RETURN_VALUE][iter_arg->name().c_str()].toVariant().toString().isEmpty())
                    {
                        iter_arg->value(resultValues[JOB_RETURN_VALUE].toObject()[iter_arg->name().c_str()].toVariant().toString().toStdString());
                    }
                }
                this->m_configuration->setCustomRh(rh_reinforcement, RH_BR_OPERATE_DATA_LAST);
            }

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

                // 首次加固修改保存的历史操作文件，改为获取到的值,本身不符合才需要获取当前值，若为符合则直接使用默认值
                if (m_isFristReinfoceFinish)
                {
                    auto rh_frist = this->m_configuration->readRhFromFile(RH_BR_OPERATE_DATA_FIRST);

                    auto& reinforcements = rh_frist->reinforcement();
                    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
                    {
                        CONTINUE_IF_TRUE(iter->name() != reinforcementResult.name());
                        KLOG_DEBUG() << "iter->name() = " << iter->name().c_str() << ", reinforcementResult.name =" << reinforcementResult.name().c_str() << "suscess";
                        auto& iter_args = iter->arg();
                        for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                        {
                            if (!resultValues[JOB_RETURN_VALUE][iter_arg->name().c_str()].toVariant().toString().isEmpty())
                                iter_arg->value(resultValues[JOB_RETURN_VALUE].toObject()[iter_arg->name().c_str()].toVariant().toString().toStdString());
                        }
                    }
                    this->m_configuration->writeRhToFile(rh_frist, RH_BR_OPERATE_DATA_FIRST);
                }
            }
            reinforcementResult.state(int32_t(state));
            scanResult.reinforcement().push_back(std::move(reinforcementResult));
            ++itemCount;
        }

        if (m_isScanFlag)
        {
            std::ostringstream ostring_stream;
            Protocol::br_job_result(ostring_stream, scanResult);
            emit ScanProgress(QString(ostring_stream.str().c_str()));
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }
}

void BRDBus::onReinforceProcessChangedCb(const JobResult& jobResult)
{
    KLOG_DEBUG("onReinforceProcessChangedCb");

    Protocol::JobResult reinforceResult(0, 0, 0);

    try
    {
        reinforceResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
        reinforceResult.job_id(jobResult.job_id);
        reinforceResult.job_state(this->m_reinforceJob->getState());

        int32_t itemCount = 0;
        for (auto iter = jobResult.running_operations.begin(); iter != jobResult.running_operations.end(); ++iter)
        {
            auto operation = this->m_reinforceJob->getOperation(*iter);
            Protocol::ReinforcementResult reinforcementResult(std::string(), 0);

            reinforcementResult.name(operation->reinforcement_name.toStdString());
            reinforcementResult.state(BRReinforcementState::BR_REINFORCEMENT_STATE_REINFORCING);
            reinforceResult.reinforcement().push_back(std::move(reinforcementResult));
            ++itemCount;
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
            ++itemCount;
        }
        // 回退中，不关注进程信息
        if (BR_FALLBACK_STATUS_IN_PROGRESS != this->m_configuration->getFallbackStatus())
        {
            std::ostringstream ostring_stream;
            Protocol::br_job_result(ostring_stream, reinforceResult);
            emit ReinforceProgress(QString(ostring_stream.str().c_str()));
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }
}

bool BRDBus::onResourceMonitor()
{
    KLOG_DEBUG("onResourceMonitor.");
    if (m_configuration->getResourceMonitorStatus() == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN)
        m_resourceMonitor->startMonitor();
    else if (m_configuration->getResourceMonitorStatus() == BRResourceMonitor::BR_RESOURCE_MONITOR_CLOSE)
        m_resourceMonitor->closeMonitor();
    else
        m_resourceMonitor->startMonitor();
    return true;
}

void KS::BRDaemon::BRDBus::scanProgressFinished()
{
    m_isScanFlag = true;
    // 回退中，不关注扫描完成
    if (BR_FALLBACK_STATUS_IN_PROGRESS != this->m_configuration->getFallbackStatus())
    {
        emit ProgressFinished();
    }
}

void KS::BRDaemon::BRDBus::reinforceProgressFinished()
{
    this->m_configuration->setFallbackStatus(BR_FALLBACK_STATUS_IS_FINISHED);
    m_isScanFlag = true;
    emit ProgressFinished();
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
