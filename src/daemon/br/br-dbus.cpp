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
#include <fstream>
#include <iostream>
#include "br-protocol.hxx"
#include "categories.h"
#include "configuration.h"
#include "plugins.h"
#include "src/daemon/br_adaptor.h"
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

BRDBus::BRDBus(QObject* parent) : timer(nullptr)
{
    this->m_dbus = new BRAdaptor(this);
}

BRDBus::~BRDBus()
{
    if (this->timer)
    {
        QObject::disconnect(this->timer, SIGNAL(QTimer::timeout()), this, SLOT(BRDBus::onResourceMonitor()));
        delete this->timer;
    }
}

BRDBus* BRDBus::instance_ = NULL;
void BRDBus::globalInit(QObject* parent)
{
    instance_ = new BRDBus(parent);
    instance_->init();
}

uint BRDBus::notification_status() const
{
    return configuration_->getNotificationStatus();
}

uint BRDBus::resource_monitor() const
{
    return configuration_->getResourceMonitorStatus();
}

uint BRDBus::standard_type() const
{
    return configuration_->getStandardType();
}

uint BRDBus::strategy_type() const
{
    return configuration_->getStrategyType();
}

uint BRDBus::time_scan() const
{
    return configuration_->getTimeScan();
}

QString BRDBus::version() const
{
    return PROJECT_VERSION;
}

void BRDBus::SetStandardType(const uint32_t& standard_type)
{
    KLOG_INFO("SetStandardType. standard type: %d.", standard_type);

    if (standard_type >= BRStandardType::BR_STANDARD_TYPE_LAST)
    {
        sendErrorReply(QDBusError::InvalidArgs, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_STANDARD_TYPE_INVALID));
    }

    RETURN_IF_TRUE(standard_type == this->configuration_->getStandardType())

    if (!this->configuration_->setStandardType(BRStandardType(standard_type)))
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_STANDARD_TYPE_FAILED));
    }
}

void BRDBus::ImportCustomRS(const QString& encoded_standard)
{
    KLOG_INFO() << "Import custom reinforce standard. encoded standard: " << encoded_standard;

    BRErrorCode error_code = BRErrorCode::SUCCESS;
    if (!this->configuration_->setCustomRs(encoded_standard, error_code))
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(error_code));
    }
}

void BRDBus::SetStrategyType(const uint32_t& strategy_type)
{
    KLOG_INFO("SetStrategyType. strategy type: %d.", strategy_type);

    if (strategy_type >= BRStrategyType::BR_STRATEGY_TYPE_LAST)
    {
        sendErrorReply(QDBusError::InvalidArgs, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_STRATEGY_TYPE_INVALID));
    }
    RETURN_IF_TRUE(strategy_type == this->configuration_->getStrategyType())

    if (!this->configuration_->setStrategyType(BRStrategyType(strategy_type)))
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_STRATEGY_TYPE_FAILED));
    }
}

void BRDBus::SetTimeScan(const uint32_t& time_scan)
{
    KLOG_INFO("Set time scan: %d.", time_scan);

    RETURN_IF_TRUE(time_scan == uint32_t(this->configuration_->getTimeScan()))

    if (!this->configuration_->setTimeScan(int(time_scan)))
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_TIME_SCAN_FAILED));
    }
}

void BRDBus::SetNotificationStatus(const uint32_t& notification_status)
{
    KLOG_INFO("Set notification status: %d.", notification_status);

    if (notification_status >= BRNotificationStatus::BR_NOTIFICATION_OTHER)
    {
        sendErrorReply(QDBusError::InvalidArgs,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_NOTIFICATION_STATUS_INVALID));
    }
    RETURN_IF_TRUE(notification_status == this->configuration_->getNotificationStatus())

    if (!this->configuration_->setNotificationStatus(BRNotificationStatus(notification_status)))
    {
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SET_NOTIFICATION_STATUS_FAILED));
    }
}

void BRDBus::ImportCustomRA(const QString& encoded_strategy)
{
    KLOG_DEBUG() << "Import custom reinforce strategy. encoded strategy: " << encoded_strategy;
    try
    {
        std::ofstream ofs(CUSTOM_RA_STRATEGY_FILEPATH, std::ios_base::out);
        ofs << encoded_strategy.toStdString();
        ofs.close();
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        return;
    }
    if (!configuration_->checkRaStrategy())
    {
        remove(CUSTOM_RA_STRATEGY_FILEPATH);
        // 不知道选选哪个错误码，所以选择了 ERROR_FAILED
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_FAILED));
    }
}

void BRDBus::SetCheckBox(const QString& reinforcement_name, const bool& checkbox_status)
{
    KLOG_DEBUG() << "reinforcement_name: " << reinforcement_name << ", SetCheckBox";
    configuration_->setRaCheckbox(reinforcement_name, checkbox_status);
}

void BRDBus::SetResourceMonitorSwitch(const uint32_t& resource_monitor)
{
    KLOG_INFO("SetResourceMonitorSwitch. resource monitor: %d.", resource_monitor);

    if (resource_monitor >= BRResourceMonitor::BR_RESOURCE_MONITOR_OR)
    {
        sendErrorReply(QDBusError::InvalidArgs, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_RESOURCE_MONITOR_INVALID));
    }

    RETURN_IF_TRUE(resource_monitor == this->configuration_->getResourceMonitorStatus())

    if (this->configuration_->setResourceMonitorStatus(BRResourceMonitor(resource_monitor)))
    {
        if (this->timer)
        {
            QObject::disconnect(this->timer, SIGNAL(timeout()), this, SLOT(BRDBus::onResourceMonitor()));
            delete this->timer;
        }
        if (BRResourceMonitor(resource_monitor) != BRResourceMonitor::BR_RESOURCE_MONITOR_CLOSE)
        {
            this->timer = new QTimer();
            timer->setInterval(RESOURCEMONITORMS);
            QObject::connect(this->timer, SIGNAL(QTimer::timeout()), this, SLOT(BRDBus::onResourceMonitor()));
            this->timer->start();
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

    auto categories = this->categories_->getCategories();

    jsonObject["item_count"] = int32_t(categories.size());
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
    auto rs = this->configuration_->getRs();

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

    auto reinforcements = this->plugins_->getReinforcements();
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

    this->configuration_->delAllCustomRa();
}

QString BRDBus::GetReinforcement(const QString& name)
{
    KLOG_DEBUG() << "GetReinforcement";

    std::ostringstream ostring_stream;
    auto reinforcement = this->plugins_->getReinforcement(name);
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

void BRDBus::SetReinforcement(const QString& reinforcement_xml)
{
    KLOG_DEBUG() << "Set reinforcement parameters, reinforcement_xml : " << reinforcement_xml;

    try
    {
        std::istringstream istring_stream(reinforcement_xml.toStdString());
        auto rs_reinforcement = Protocol::br_reinforcement(istring_stream, xml_schema::Flags::dont_validate);
        if (!this->configuration_->setCustomRa(*rs_reinforcement.get()))
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
    this->configuration_->delCustomRa(name);
}

qlonglong BRDBus::Scan(const QStringList& names)
{
    KLOG_INFO() << "Scan. range : " << names.join(" ").toLocal8Bit();

    // 已经在扫描则返回错误
    if (this->scan_job_ && this->scan_job_->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        sendErrorReply(QDBusError::InternalError, BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SCAN_IS_RUNNING));
        // THROW_DBUSCXX_ERROR(BRErrorCode::ERROR_DAEMON_SCAN_IS_RUNNING);
    }

    try
    {
        this->scan_job_ = Job::create();
        if (!is_frist_reinfoce_)
            is_frist_reinfoce_finish_ = false;
        for (auto iter = names.begin(); iter != names.end(); ++iter)
        {
            auto& name = (*iter);
            auto reinforcement = this->plugins_->getReinforcement(name);

            if (!reinforcement)
            {
                sendErrorReply(QDBusError::InternalError,
                               BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND));
            }

            auto reinforcement_interface = this->plugins_->getReinforcementInterface(reinforcement->getPluginName(),
                                                                                     reinforcement->getName());
            if (!reinforcement_interface)
            {
                sendErrorReply(QDBusError::InternalError,
                               BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND));
            }

            this->scan_job_->addOperation(reinforcement->getPluginName(),
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

    QObject::connect(this->scan_job_.get(), &Job::process_finished_, this, &BRDBus::scanProgressFinished);
    QObject::connect(this->scan_job_.get(), &Job::process_changed_, this, &BRDBus::onScanProcessChangedCb);

    if (!this->scan_job_->runAsync())
    {
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_SCAN_ALL_JOB_FAILED));
    }
    return this->scan_job_->getId();
}

qlonglong BRDBus::Reinforce(const QStringList& names)
{
    KLOG_INFO() << "Reinforce. range : " << names.join(" ").toLocal8Bit();
    is_scan_flag_ = false;
    // 已经在加固则返回错误
    if (this->reinforce_job_ && this->reinforce_job_->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCE_IS_RUNNING));
    }

    try
    {
        this->reinforce_job_ = Job::create();

        // 首次加固保存所有加固前配置
        is_frist_reinfoce_finish_ = false;

        if (is_frist_reinfoce_)
        {
            QStringList names_rh;
            auto rh = this->configuration_->readRhFromFile(RH_BR_OPERATE_DATA_FIRST);
            if (rh->reinforcement().empty())
            {
                auto reinforcements = this->plugins_->getReinforcements();
                for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
                {
                    auto& rs_reinforcement = (*iter)->getRs();
                    names_rh.push_back(QString::fromStdString(rs_reinforcement.name()));
                    rh->reinforcement().push_back(rs_reinforcement);
                }
                this->configuration_->writeRhToFile(rh, RH_BR_OPERATE_DATA_FIRST);

                is_frist_reinfoce_finish_ = true;
                Scan(names_rh);
            }
            // 此处需等待扫描进程完成后置为false
            is_frist_reinfoce_ = false;
        }
        else
        {
            KLOG_DEBUG("Reinforce is_scan_flag_ = %d", is_scan_flag_);
            Scan(names);
        }

        for (auto iter = names.begin(); iter != names.end(); ++iter)
        {
            auto& name = (*iter);
            auto reinforcement = this->plugins_->getReinforcement(name);
            if (!reinforcement)
            {
                sendErrorReply(QDBusError::InternalError,
                               BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND));
            }

            auto reinforcement_interface = this->plugins_->getReinforcementInterface(reinforcement->getPluginName(),
                                                                                     reinforcement->getName());
            if (!reinforcement_interface)
            {
                sendErrorReply(QDBusError::InternalError,
                               BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND));
            }

            QJsonObject param;
            QString param_str = "";

            if (snapshot_status_ == BRSnapshotStatus::BR_INITIAL_STATUS)
            {
                auto rh = this->configuration_->readRhFromFile(RH_BR_OPERATE_DATA_FIRST);
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
            else if (snapshot_status_ == BRSnapshotStatus::BR_LAST_REINFORCEMENT_STATUS)
            {
                //
                auto rh = this->configuration_->readRhFromFile(RH_BR_OPERATE_DATA_FIRST);
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
                auto rh_last = this->configuration_->readRhFromFile(RH_BR_OPERATE_DATA_LAST);
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

            this->reinforce_job_->addOperation(reinforcement->getPluginName(),
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
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_DAEMON_REINFORCE_RANGE_INVALID));
    }

    QObject::connect(this->reinforce_job_.get(), &Job::process_changed_, this, &BRDBus::onReinfoceProcessChangedCb);
    QObject::connect(this->reinforce_job_.get(), &Job::process_finished_, this, &BRDBus::reinfoceProgressFinished);

    if (!this->reinforce_job_->runAsync())
    {
        sendErrorReply(QDBusError::InternalError,
                       BR_ERROR2STR(BRErrorCode::ERROR_CORE_REINFORCE_JOB_FAILED));
    }

    return this->reinforce_job_->getId();
}

void BRDBus::Cancel(const qlonglong& job_id)
{
    KLOG_INFO("Cancel. job id: %lld.", job_id);

    BRErrorCode error_code = BRErrorCode::SUCCESS;

    if (this->scan_job_ &&
        job_id == this->scan_job_->getId() &&
        this->scan_job_->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        if (!this->scan_job_->cancel())
        {
            error_code = BRErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_1;
        }
    }
    else if (this->reinforce_job_ &&
             job_id == this->reinforce_job_->getId() &&
             this->reinforce_job_->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        if (!this->reinforce_job_->cancel())
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

void BRDBus::SetFallback(const uint32_t& snapshot_status)
{
    KLOG_INFO("Set fallback. snapshot_status: %d.", snapshot_status);
    is_reinfoce_flag_ = false;
    QStringList names_rh;
    auto rh = this->configuration_->readRhFromFile(RH_BR_OPERATE_DATA_FIRST);
    auto& rh_reinforcements = rh->reinforcement();
    for (auto iter = rh_reinforcements.begin(); iter != rh_reinforcements.end(); ++iter)
    {
        names_rh.push_back(QString::fromStdString(iter->name()));
    }

    if (snapshot_status == BRSnapshotStatus::BR_INITIAL_STATUS)
    {
        snapshot_status_ = BRSnapshotStatus::BR_INITIAL_STATUS;
        if (names_rh.empty())
        {
            // 需回退的加固项为空 不需要进行加固了 Reinforce Finish
            emit ProgressFinished();
            is_reinfoce_flag_ = true;
            return;
        }
        Reinforce(names_rh);

        snapshot_status_ = BRSnapshotStatus::BR_OTHER_STATUS;
    }
    else if (snapshot_status == BRSnapshotStatus::BR_LAST_REINFORCEMENT_STATUS)
    {
        snapshot_status_ = BRSnapshotStatus::BR_LAST_REINFORCEMENT_STATUS;
        if (names_rh.empty())
        {
            // 需回退的加固项为空 不需要进行加固了 Reinforce Finish
            emit ProgressFinished();
            is_reinfoce_flag_ = true;
            return;
        }
        Reinforce(names_rh);

        snapshot_status_ = BRSnapshotStatus::BR_OTHER_STATUS;
    }
    else
    {
        snapshot_status_ = BRSnapshotStatus::BR_OTHER_STATUS;
        is_reinfoce_flag_ = true;
    }
}

void BRDBus::init()
{
    this->configuration_ = Configuration::getInstance();
    this->categories_ = Categories::getInstance();
    this->plugins_ = Plugins::getInstance();

    QDBusConnection dbusConnection = QDBusConnection::systemBus();
    if (!dbusConnection.registerObject(BR_DBUS_OBJECT_PATH, this))
    {
        KLOG_ERROR() << "register Service error:" << dbusConnection.lastError().message();
    }

    this->resource_monitor_ = new ResourceMonitor();
    KLOG_DEBUG("init ResourceMonitor.");
    QObject::connect(this->resource_monitor_, &ResourceMonitor::homeFreeSpaceRatio_,
                     this, &BRDBus::homeFreeSpaceRatio);
    QObject::connect(this->resource_monitor_, &ResourceMonitor::rootFreeSpaceRatio_,
                     this, &BRDBus::rootFreeSpaceRatio);
    QObject::connect(this->resource_monitor_, &ResourceMonitor::cpuAverageLoadRatio_,
                     this, &BRDBus::cpuAverageLoadRatio);
    QObject::connect(this->resource_monitor_, &ResourceMonitor::vmstatSiso_, this, &BRDBus::vmstatSiSo);

    if (configuration_->getResourceMonitorStatus() == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN)
    {
        timer = new QTimer();
        timer->setInterval(RESOURCEMONITORMS);
        QObject::connect(this->timer, &QTimer::timeout, this, &BRDBus::onResourceMonitor);
    }
}

void BRDBus::onScanProcessChangedCb(const JobResult& job_result)
{
    KLOG_DEBUG() << "onScanProcessChangedCb";

    Protocol::JobResult scan_result(0, 0, 0);

    try
    {
        scan_result.process(job_result.finished_operation_num * 100.0 / job_result.sum_operation_num);
        scan_result.job_id(job_result.job_id);
        scan_result.job_state(this->scan_job_->getState());

        int32_t item_count = 0;
        for (auto iter = job_result.running_operations.begin(); iter != job_result.running_operations.end(); ++iter)
        {
            auto operation = this->scan_job_->getOperation((*iter));

            Protocol::ReinforcementResult reinforcement_result(std::string(), 0);
            reinforcement_result.name(operation->reinforcement_name.toStdString());
            reinforcement_result.state(BRReinforcementState::BR_REINFORCEMENT_STATE_SCANNING);
            reinforcement_result.args("");
            scan_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }

        for (auto iter = job_result.current_finished_operations.begin(); iter != job_result.current_finished_operations.end(); ++iter)
        {
            auto& operation_result = (*iter);
            auto operation = this->scan_job_->getOperation(operation_result.operation_id);
            Protocol::ReinforcementResult reinforcement_result(std::string(), 0);

            reinforcement_result.name(operation->reinforcement_name.toStdString());

            BRReinforcementState state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNKNOWN;
            const auto result_values = StrUtils::str2jsonObject(operation_result.result);
            // 如果结果为空应该时任务被取消了，如果在收到客户端的任务取消命令时操作已经在执行，结果也可能不为空，所以这里不能通过任务是否被取消的状态来判断
            if (result_values.isEmpty())
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNSCAN;
                reinforcement_result.args("");
            }
            else if (result_values[JOB_ERROR_STR].isString())
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_SCAN_ERROR;
                reinforcement_result.args("");
                reinforcement_result.error(result_values[JOB_ERROR_STR].toString().toStdString());
            }
            else
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_SCAN_DONE;
                reinforcement_result.args(StrUtils::json2str(result_values).toStdString());
            }
            auto reinforcement = this->plugins_->getReinforcement(operation->reinforcement_name);

            // 上一次历史操作存入rh文件
            if (!is_scan_flag_)
            {
                auto rh_reinforcement = reinforcement->getRs();
                auto& iter_args = rh_reinforcement.arg();
                for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                {
                    if (!result_values[JOB_RETURN_VALUE][iter_arg->name().c_str()].toString().isEmpty())
                        iter_arg->value(StrUtils::json2str(result_values[JOB_RETURN_VALUE][iter_arg->name().c_str()].toObject()).toStdString());
                    KLOG_DEBUG() << "fix arg StrUtils::json2str(result_values[JOB_RETURN_VALUE][i]) = "
                                 << StrUtils::json2str(result_values[JOB_RETURN_VALUE][iter_arg->name().c_str()].toObject()).toLocal8Bit()
                                 << "iter_arg : name: " << iter_arg->name().c_str()
                                 << " value: "
                                 << iter_arg->value().c_str();
                }
                this->configuration_->setCustomRh(rh_reinforcement, RH_BR_OPERATE_DATA_LAST);
            }

            if ((state & BRReinforcementState::BR_REINFORCEMENT_STATE_SCAN_DONE) != 0 &&
                reinforcement &&
                reinforcement->matchRules(result_values[JOB_RETURN_VALUE].toObject()))
            {
                state = BRReinforcementState(state | BRReinforcementState::BR_REINFORCEMENT_STATE_SAFE);
            }
            else
            {
                state = BRReinforcementState(state | BRReinforcementState::BR_REINFORCEMENT_STATE_UNSAFE);

                // 首次加固修改保存的历史操作文件，改为获取到的值
                if (is_frist_reinfoce_finish_)
                {
                    auto rh_frist = this->configuration_->readRhFromFile(RH_BR_OPERATE_DATA_FIRST);
                    //                    KLOG_DEBUG("is_frist_reinfoce_finish_ : %s", RH_BR_OPERATE_DATA_FIRST);

                    auto& reinforcements = rh_frist->reinforcement();
                    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
                    {
                        CONTINUE_IF_TRUE(iter->name() != reinforcement_result.name());
                        KLOG_DEBUG() << "iter->name() = " << iter->name().c_str() << ", reinforcement_result.name =" << reinforcement_result.name().c_str() << "suscess";
                        auto& iter_args = iter->arg();
                        for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                        {
                            if (!result_values[JOB_RETURN_VALUE][iter_arg->name().c_str()].toString().isEmpty())
                                iter_arg->value(StrUtils::json2str(result_values[JOB_RETURN_VALUE][iter_arg->name().c_str()].toObject()).toStdString());
                            // KLOG_DEBUG("iter_arg : name : %s value : %s", iter_arg->name().c_str(), iter_arg->value().c_str());
                        }
                    }
                    this->configuration_->writeRhToFile(rh_frist, RH_BR_OPERATE_DATA_FIRST);
                }
            }
            reinforcement_result.state(int32_t(state));
            scan_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }

        if (is_scan_flag_)
        {
            std::ostringstream ostring_stream;
            Protocol::br_job_result(ostring_stream, scan_result);
            emit ScanProgress(QString(ostring_stream.str().c_str()));
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }
}

void BRDBus::onReinfoceProcessChangedCb(const JobResult& job_result)
{
    KLOG_DEBUG("onReinfoceProcessChangedCb");

    Protocol::JobResult reinforce_result(0, 0, 0);

    try
    {
        reinforce_result.process(job_result.finished_operation_num * 100.0 / job_result.sum_operation_num);
        reinforce_result.job_id(job_result.job_id);
        reinforce_result.job_state(this->reinforce_job_->getState());

        int32_t item_count = 0;
        for (auto iter = job_result.running_operations.begin(); iter != job_result.running_operations.end(); ++iter)
        {
            auto operation = this->reinforce_job_->getOperation(*iter);
            Protocol::ReinforcementResult reinforcement_result(std::string(), 0);

            reinforcement_result.name(operation->reinforcement_name.toStdString());
            reinforcement_result.state(BRReinforcementState::BR_REINFORCEMENT_STATE_REINFORCING);
            reinforce_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }

        for (auto iter = job_result.current_finished_operations.begin(); iter != job_result.current_finished_operations.end(); ++iter)
        {
            auto& operation_result = (*iter);
            auto operation = this->reinforce_job_->getOperation(operation_result.operation_id);
            Protocol::ReinforcementResult reinforcement_result(std::string(), 0);

            reinforcement_result.name(operation->reinforcement_name.toStdString());

            BRReinforcementState state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNKNOWN;
            auto result_values = StrUtils::str2jsonValue(operation_result.result);

            if (result_values.isNull())
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNREINFORCE;
            }
            else if (result_values[JOB_ERROR_STR].isString())
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_REINFORCE_ERROR;
                reinforcement_result.error(result_values[JOB_ERROR_STR].toString().toStdString());
            }
            else
            {
                state = BRReinforcementState::BR_REINFORCEMENT_STATE_REINFORCE_DONE;
            }
            reinforcement_result.state(int32_t(state));
            reinforce_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }
        if (is_reinfoce_flag_)
        {
            std::ostringstream ostring_stream;
            Protocol::br_job_result(ostring_stream, reinforce_result);
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
    if (configuration_->getResourceMonitorStatus() == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN)
        resource_monitor_->startMonitor();
    else if (configuration_->getResourceMonitorStatus() == BRResourceMonitor::BR_RESOURCE_MONITOR_CLOSE)
        resource_monitor_->closeMonitor();
    else
        resource_monitor_->startMonitor();
    return true;
}

void BRDBus::homeFreeSpaceRatio(const float space_ratio)
{
    // 家目录可用空间小于10%告警
    float homeSpa = 0.1;
    if (space_ratio < homeSpa)
    {
        KLOG_WARNING() << "home free space less than 10%. homeFreeSpaceRatio " << space_ratio;
        _audit_log(1101, -1, "home free space less than 10%.");
        emit HomeFreeSpaceRatioLower(QString(std::to_string(space_ratio).c_str()));
    }
}

void BRDBus::rootFreeSpaceRatio(const float space_ratio)
{
    // 根目录可用空间小于10%告警
    float rootSpa = 0.1;
    if (space_ratio < rootSpa)
    {
        KLOG_WARNING() << "root free space less than 10%. rootFreeSpaceRatio " << space_ratio;
        _audit_log(1101, -1, "root free space less than 10%.");
        emit RootFreeSpaceRatioLower(QString(std::to_string(space_ratio).c_str()));
    }
}

void BRDBus::cpuAverageLoadRatio(const float load_ratio)
{
    // cpu单核五分钟平均负载大于1告警
    float cpuLoad = 1;
    if (load_ratio >= cpuLoad)
    {
        KLOG_WARNING("The average load of a single core CPU exceeds 1.");
        KLOG_WARNING("cpuAverageLoadRatio %f.", load_ratio);

        _audit_log(1101, -1, "The average load of a single core CPU exceeds 1.");

        emit CpuAverageLoadRatioHigher(QString(std::to_string(load_ratio).c_str()));
    }
}

void BRDBus::vmstatSiSo(const QVector<QString> results)
{
    // vmstat swap 中si或者so不为0告警
    QString si(results.at(0));
    QString so(results.at(1));
    if (si != "0")
    {
        KLOG_WARNING() << "The vmstat swap page si is not 0. vmstat si is " << results.at(0);
        _audit_log(1101, -1, "The vmstat swap page si is not 0.");
        emit VmstatSiSoabnormal(si, so);
    }

    if (so != "0")
    {
        KLOG_WARNING() << "The vmstat swap page so is not 0. vmstat so is " << results.at(1);
        _audit_log(1101, -1, "The vmstat swap page so is not 0.");
        if (si == "0")
            emit VmstatSiSoabnormal(si, so);
    }
}

}  // namespace BRDaemon
}  // namespace KS
