/**
 * @file          /ks-ssr-manager/src/daemon/dbus.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/dbus.h"
#include <json/json.h>
#include <kylin-license/license-i.h>
#include <libaudit.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "src/daemon/categories.h"
#include "src/daemon/configuration.h"
#include "src/daemon/plugins.h"
#include "src/daemon/ssr-protocol.hxx"
#include "src/daemon/utils.h"

namespace KS
{
namespace Daemon
{
// 一分钟
#define RESOURCEMONITORMS 1000 * 60 * 1

#define JOB_ERROR_STR "error"
#define JOB_RETURN_VALUE "return_value"
// #define CUSTOM_RA_FILEPATH SSR_INSTALL_DATADIR "/ssr-custom-ra.xml"
#define CUSTOM_RA_STRATEGY_FILEPATH SSR_INSTALL_DATADIR "/ssr-custom-ra-strategy.xml"
#define RH_SSR_OPERATE_DATA_FIRST SSR_INSTALL_DATADIR "/ssr-rh-first.xml"
#define RH_SSR_OPERATE_DATA_LAST SSR_INSTALL_DATADIR "/ssr-rh-last.xml"

// static std::shared_ptr<SSRReinforcementInterface> reinforcement_interface_;
// static std::string param_str_ = "";

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

DBus::DBus(::DBus::Connection& connection) : ::DBus::ObjectAdaptor(connection, SSR_DBUS_OBJECT_PATH),
                                             dbus_connection_(connection)
{
}

DBus::~DBus()
{
    if (this->timeout_handler_)
    {
        this->timeout_handler_.disconnect();
    }
}

DBus* DBus::instance_ = NULL;
void DBus::global_init(::DBus::Connection& connection)
{
    instance_ = new DBus(connection);
    instance_->init();
}

void DBus::SetStandardType(const uint32_t& standard_type)
{
    KLOG_INFO("SetStandardType. standard type: %d.", standard_type);
    KLOG_PROFILE("standard type: %d.", standard_type);

    if (standard_type >= SSRStandardType::SSR_STANDARD_TYPE_LAST)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_STANDARD_TYPE_INVALID);
    }

    RETURN_IF_TRUE(standard_type == this->configuration_->get_standard_type())

    if (!this->configuration_->set_standard_type(SSRStandardType(standard_type)))
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SET_STANDARD_TYPE_FAILED);
    }
}

void DBus::ImportCustomRS(const std::string& encoded_standard)
{
    KLOG_INFO("Import custom reinforce standard.");
    KLOG_PROFILE("encoded standard: %s.", encoded_standard.c_str());

    SSRErrorCode error_code = SSRErrorCode::SUCCESS;
    if (!this->configuration_->set_custom_rs(encoded_standard, error_code))
    {
        THROW_DBUSCXX_ERROR(error_code);
    }
}

void DBus::SetStrategyType(const uint32_t& strategy_type)
{
    KLOG_INFO("SetStrategyType. strategy type: %d.", strategy_type);
    KLOG_DEBUG("strategy type: %d.", strategy_type);

    if (strategy_type >= SSRStrategyType::SSR_STRATEGY_TYPE_LAST)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_STRATEGY_TYPE_INVALID);
    }
    RETURN_IF_TRUE(strategy_type == this->configuration_->get_strategy_type())

    if (!this->configuration_->set_strategy_type(SSRStrategyType(strategy_type)))
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SET_STRATEGY_TYPE_FAILED);
    }
}

void DBus::SetTimeScan(const uint32_t& time_scan)
{
    KLOG_INFO("Set time scan: %d.", time_scan);

    RETURN_IF_TRUE(time_scan == uint32_t(this->configuration_->get_time_scan()))

    if (!this->configuration_->set_time_scan(int(time_scan)))
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SET_TIME_SCAN_FAILED);
    }
}

void DBus::SetNotificationStatus(const uint32_t& notification_status)
{
    KLOG_INFO("Set notification status: %d.", notification_status);

    if (notification_status >= SSRNotificationStatus::SSR_NOTIFICATION_OTHER)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_NOTIFICATION_STATUS_INVALID);
    }
    RETURN_IF_TRUE(notification_status == this->configuration_->get_notification_status())

    if (!this->configuration_->set_notification_status(SSRNotificationStatus(notification_status)))
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SET_NOTIFICATION_STATUS_FAILED);
    }
}

void DBus::ImportCustomRA(const std::string& encoded_strategy)
{
    KLOG_INFO("Import custom reinforce strategy.");
    KLOG_DEBUG("encoded strategy: %s.", encoded_strategy.c_str());
    try
    {
        std::ofstream ofs(CUSTOM_RA_STRATEGY_FILEPATH, std::ios_base::out);
        //        ssr_ra(ofs, *ra.get());
        ofs << encoded_strategy;
        ofs.close();
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        return;
    }
    SSRErrorCode error_code = SSRErrorCode::SUCCESS;
    if (!configuration_->check_ra_strategy())
    {
        remove(CUSTOM_RA_STRATEGY_FILEPATH);
        THROW_DBUSCXX_ERROR(error_code);
    }
}

void DBus::SetCheckBox(const std::string& reinforcement_name, const bool& checkbox_status)
{
    KLOG_DEBUG("reinforcement_name: %s, SetCheckBox", reinforcement_name.c_str());
    configuration_->set_ra_checkbox(reinforcement_name, checkbox_status);
}

void DBus::SetResourceMonitorSwitch(const uint32_t& resource_monitor)
{
    KLOG_INFO("SetResourceMonitorSwitch. resource monitor: %d.", resource_monitor);

    if (resource_monitor >= SSRResourceMonitor::SSR_RESOURCE_MONITOR_OR)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_RESOURCE_MONITOR_INVALID);
    }

    RETURN_IF_TRUE(resource_monitor == this->configuration_->get_resource_monitor_status())

    if (this->configuration_->set_resource_monitor_status(SSRResourceMonitor(resource_monitor)))
    {
        if (this->timeout_handler_)
            this->timeout_handler_.disconnect();
        if (SSRResourceMonitor(resource_monitor) != SSRResourceMonitor::SSR_RESOURCE_MONITOR_CLOSE)
        {
            auto timeout = Glib::MainContext::get_default()->signal_timeout();
            this->timeout_handler_ = timeout.connect(sigc::mem_fun(this, &DBus::on_resource_monitor), RESOURCEMONITORMS);
        }
    }
    else
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SET_RESOURCE_MONITOR_FAILED);
    }
}

std::string DBus::GetCategories()
{
    KLOG_PROFILE("");

    Json::Value values;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "";

    auto categories = this->categories_->get_categories();

    try
    {
        values["item_count"] = int32_t(categories.size());
        for (uint32_t i = 0; i < categories.size(); ++i)
        {
            auto category = categories[i];

            values["items"][i]["name"] = category->name;
            values["items"][i]["label"] = category->label;
            values["items"][i]["description"] = category->description;
            values["items"][i]["icon_name"] = category->icon_name;
        }

        return Json::writeString(wbuilder, values);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_CONVERT_CATEGORIES2JSON_FAILED);
    }

    return std::string();
}

std::string DBus::GetRS()
{
    std::ostringstream ostring_stream;
    auto rs = this->configuration_->get_rs();

    if (!rs)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_GET_RS_FAILED);
    }

    try
    {
        Protocol::ssr_rs(ostring_stream, *rs.get());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_GET_RS_FAILED);
    }
    return ostring_stream.str();
}

std::string DBus::GetReinforcements()
{
    KLOG_PROFILE("");

    std::ostringstream ostring_stream;
    Protocol::Reinforcements protocol_reinforcements;

    auto reinforcements = this->plugins_->get_reinforcements();
    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
    {
        auto& rs_reinforcement = (*iter)->get_rs();
        protocol_reinforcements.reinforcement().push_back(rs_reinforcement);
    }

    try
    {
        Protocol::ssr_reinforcements(ostring_stream, protocol_reinforcements);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENT_FAILED);
    }
    return ostring_stream.str();
}

void DBus::ResetReinforcements()
{
    KLOG_INFO("Reset all reinforcement parameters.");
    KLOG_PROFILE("");

    this->configuration_->del_all_custom_ra();
}

std::string DBus::GetReinforcement(const std::string& name)
{
    KLOG_PROFILE("");

    std::ostringstream ostring_stream;
    auto reinforcement = this->plugins_->get_reinforcement(name);
    if (!reinforcement)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND);
    }
    auto& rs_reinforcement = reinforcement->get_rs();

    try
    {
        Protocol::ssr_reinforcement(ostring_stream, rs_reinforcement);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENT_FAILED);
    }
    return ostring_stream.str();
}

void DBus::SetReinforcement(const std::string& reinforcement_xml)
{
    KLOG_INFO("Set reinforcement parameters.");
    KLOG_PROFILE("reinforcement_xml : %s", reinforcement_xml.c_str());

    try
    {
        std::istringstream istring_stream(reinforcement_xml);
        auto rs_reinforcement = Protocol::ssr_reinforcement(istring_stream, xml_schema::Flags::dont_validate);
        if (!this->configuration_->set_custom_ra(*rs_reinforcement.get()))
        {
            THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SET_REINFORCEMENT_FAILED);
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SET_REINFORCEMENT_FAILED);
    }
}

void DBus::ResetReinforcement(const std::string& name)
{
    KLOG_INFO("Reset reinforcement parameters. name = %s", name.c_str());
    KLOG_PROFILE("");
    this->configuration_->del_custom_ra(name);
}

// static std::string ReinforcementInterfaceScanFun()
// {
//     Json::Value retval;
//     std::string args;
//     std::string error;
//     if (reinforcement_interface_->get(args, error))
//     {
//         retval[JOB_RETURN_VALUE] = StrUtils::str2json(args);
//     }
//     else
//     {
//         retval[JOB_ERROR_STR] = error;
//     }
//     return StrUtils::json2str(retval);
// }

int64_t DBus::Scan(const std::vector<std::string>& names)
{
    KLOG_INFO("Scan. range : %s", StrUtils::join(names, " ").c_str());
    //    KLOG_PROFILE("range: %s.", StrUtils::join(names, " ").c_str());

    // 已经在扫描则返回错误
    if (this->scan_job_ && this->scan_job_->get_state() == SSRJobState::SSR_JOB_STATE_RUNNING)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SCAN_IS_RUNNING);
    }

    try
    {
        this->scan_job_ = Job::create();
        if (!is_frist_reinfoce_)
            is_frist_reinfoce_finish_ = false;
        for (auto iter = names.begin(); iter != names.end(); ++iter)
        {
            auto& name = (*iter);
            auto reinforcement = this->plugins_->get_reinforcement(name);

            if (!reinforcement)
            {
                THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND, name);
            }

            auto reinforcement_interface = this->plugins_->get_reinfocement_interface(reinforcement->get_plugin_name(),
                                                                                      reinforcement->get_name());
            if (!reinforcement_interface)
            {
                THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND);
            }

            this->scan_job_->add_operation(reinforcement->get_plugin_name(),
                                           reinforcement->get_name(),
                                           [reinforcement_interface]() -> std::string {
                                               Json::Value retval;
                                               std::string args;
                                               std::string error;
                                               if (reinforcement_interface->get(args, error))
                                               {
                                                   retval[JOB_RETURN_VALUE] = StrUtils::str2json(args);
                                               }
                                               else
                                               {
                                                   retval[JOB_ERROR_STR] = error;
                                               }
                                               return StrUtils::json2str(retval);
                                           });

            // 重新扫描时需要清理加固项的安全状态
            //             reinforcement->state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_UNKNOWN;
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SCAN_RANGE_INVALID);
    }

    this->scan_job_->signal_process_finished().connect(sigc::mem_fun(this, &DBus::scan_progress_finished));
    this->scan_job_->signal_process_changed().connect(sigc::mem_fun(this, &DBus::on_scan_process_changed_cb));

    if (!this->scan_job_->run_async())
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SCAN_ALL_JOB_FAILED);
    }

    //    is_frist_scan_ = false;
    return this->scan_job_->get_id();
}

// static std::string ReinforcementInterfaceReinforceFun()
// {
//     std::string error;
//     Json::Value retval;
//     if (!reinforcement_interface_->set(param_str_, error))
//     {
//         retval[JOB_ERROR_STR] = error;
//     }
//     else
//     {
//     // 设置为空字符串，这里主要是为了区分加固成功和取消加固两种状态，后续可能会调整改逻辑
//     retval[JOB_RETURN_VALUE] = std::string();
//     }
//     return StrUtils::json2str(retval);
// }

int64_t DBus::Reinforce(const std::vector<std::string>& names)
{
    KLOG_INFO("Reinforce. range : %s", StrUtils::join(names, " ").c_str());
    //    KLOG_PROFILE("range: %s.", StrUtils::join(names, " ").c_str());
    // 未授权不允许加固
    if (this->license_values.isNull() ||
        this->license_values[LICENSE_JK_ACTIVATION_STATUS].isNull() ||
        this->license_values[LICENSE_JK_ACTIVATION_STATUS].asInt() != LicenseActivationStatus::LAS_ACTIVATED)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SOFTWARE_UNACTIVATED);
    }

    is_scan_flag_ = false;
    // 已经在加固则返回错误
    if (this->reinforce_job_ && this->reinforce_job_->get_state() == SSRJobState::SSR_JOB_STATE_RUNNING)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_REINFORCE_IS_RUNNING);
    }

    try
    {
        this->reinforce_job_ = Job::create();

        // 首次加固保存所有加固前配置
        is_frist_reinfoce_finish_ = false;

        if (is_frist_reinfoce_)
        {
            std::vector<std::string> names_rh;
            auto rh = this->configuration_->read_rh_from_file(RH_SSR_OPERATE_DATA_FIRST);
            if (rh->reinforcement().empty())
            {
                auto reinforcements = this->plugins_->get_reinforcements();
                for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
                {
                    auto& rs_reinforcement = (*iter)->get_rs();
                    names_rh.push_back(rs_reinforcement.name());
                    rh->reinforcement().push_back(rs_reinforcement);
                }
                this->configuration_->write_rh_to_file(rh, RH_SSR_OPERATE_DATA_FIRST);

                is_frist_reinfoce_finish_ = true;
                Scan(names_rh);
            }
            // 此处需等待扫描进程完成后置为false
            is_frist_reinfoce_ = false;
        }
        else
        {
            KLOG_DEBUG("Reinforce is_scan_flag_ = %d", is_scan_flag_);
            //            if (snapshot_status_ != SSR_INITIAL_STATUS)
            Scan(names);
        }

        for (auto iter = names.begin(); iter != names.end(); ++iter)
        {
            auto& name = (*iter);
            auto reinforcement = this->plugins_->get_reinforcement(name);
            if (!reinforcement)
            {
                THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND, name);
            }

            auto reinforcement_interface = this->plugins_->get_reinfocement_interface(reinforcement->get_plugin_name(),
                                                                                      reinforcement->get_name());
            if (!reinforcement_interface)
            {
                THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND);
            }

            Json::Value param;
            std::string param_str = "";

            if (snapshot_status_ == SSR_INITIAL_STATUS)
            {
                auto rh = this->configuration_->read_rh_from_file(RH_SSR_OPERATE_DATA_FIRST);
                auto& rh_reinforcements = rh->reinforcement();
                for (auto iter = rh_reinforcements.begin(); iter != rh_reinforcements.end(); ++iter)
                {
                    CONTINUE_IF_TRUE(iter->name() != name);
                    auto& iter_args = iter->arg();
                    for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                    {
                        param[iter_arg->name()] = StrUtils::str2json(iter_arg->value());
                    }
                    param_str = StrUtils::json2str(param);
                }
                KLOG_DEBUG("frist fallback name : %s", name.c_str());
                KLOG_DEBUG("frist fallback param_str : %s", param_str.c_str());
            }
            else if (snapshot_status_ == SSR_LAST_REINFORCEMENT_STATUS)
            {
                //
                auto rh = this->configuration_->read_rh_from_file(RH_SSR_OPERATE_DATA_FIRST);
                auto& rh_reinforcements = rh->reinforcement();
                for (auto iter = rh_reinforcements.begin(); iter != rh_reinforcements.end(); ++iter)
                {
                    CONTINUE_IF_TRUE(iter->name() != name);
                    auto& iter_args = iter->arg();
                    for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                    {
                        param[iter_arg->name()] = StrUtils::str2json(iter_arg->value());
                    }
                    param_str = StrUtils::json2str(param);
                }
                auto rh_last = this->configuration_->read_rh_from_file(RH_SSR_OPERATE_DATA_LAST);
                auto& rh_last_reinforcements = rh_last->reinforcement();

                for (auto iter = rh_last_reinforcements.begin(); iter != rh_last_reinforcements.end(); ++iter)
                {
                    CONTINUE_IF_TRUE(iter->name() != name);
                    auto& iter_args = iter->arg();
                    for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                    {
                        param[iter_arg->name()] = StrUtils::str2json(iter_arg->value());
                    }
                }

                param_str = StrUtils::json2str(param);
            }
            else
            {
                auto& args = reinforcement->get_rs().arg();
                for (auto arg_iter = args.begin(); arg_iter != args.end(); ++arg_iter)
                {
                    param[arg_iter->name()] = StrUtils::str2json(arg_iter->value());
                }
                param_str = StrUtils::json2str(param);
            }

            this->reinforce_job_->add_operation(reinforcement->get_plugin_name(),
                                                reinforcement->get_name(),
                                                [reinforcement_interface, param_str]() -> std::string {
                                                    std::string error;
                                                    Json::Value retval;
                                                    if (!reinforcement_interface->set(param_str, error))
                                                    {
                                                        retval[JOB_ERROR_STR] = error;
                                                    }
                                                    else
                                                    {
                                                        // 设置为空字符串，这里主要是为了区分加固成功和取消加固两种状态，后续可能会调整改逻辑
                                                        retval[JOB_RETURN_VALUE] = std::string();
                                                    }
                                                    return StrUtils::json2str(retval);
                                                });
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_REINFORCE_RANGE_INVALID);
    }

    this->reinforce_job_->signal_process_changed().connect(sigc::mem_fun(this, &DBus::on_reinfoce_process_changed_cb));
    this->reinforce_job_->signal_process_finished().connect(sigc::mem_fun(this, &DBus::reinfoce_progress_finished));

    if (!this->reinforce_job_->run_async())
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_CORE_REINFORCE_JOB_FAILED);
    }

    return this->reinforce_job_->get_id();
}

void DBus::Cancel(const int64_t& job_id)
{
    KLOG_INFO("Cancel. job id: %d.", job_id);
    //    KLOG_PROFILE("job id: %d.", job_id);

    SSRErrorCode error_code = SSRErrorCode::SUCCESS;

    if (this->scan_job_ &&
        job_id == this->scan_job_->get_id() &&
        this->scan_job_->get_state() == SSRJobState::SSR_JOB_STATE_RUNNING)
    {
        if (!this->scan_job_->cancel())
        {
            error_code = SSRErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_1;
        }
    }
    else if (this->reinforce_job_ &&
             job_id == this->reinforce_job_->get_id() &&
             this->reinforce_job_->get_state() == SSRJobState::SSR_JOB_STATE_RUNNING)
    {
        if (!this->reinforce_job_->cancel())
        {
            error_code = SSRErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_2;
        }
    }
    else
    {
        error_code = SSRErrorCode::ERROR_DAEMON_CANCEL_NOTFOUND_JOB;
    }

    if (error_code != SSRErrorCode::SUCCESS)
    {
        THROW_DBUSCXX_ERROR(error_code);
    }
}

std::string DBus::GetLicense()
{
    std::string retval;
    try
    {
        retval = this->license_object_proxy_->GetLicense();
    }
    catch (const ::DBus::Error& e)
    {
        KLOG_WARNING("%s.", e.what());
        throw e;
    }
    return retval;
}

void DBus::SetFallback(const uint32_t& snapshot_status)
{
    KLOG_INFO("Set fallback. snapshot_status: %d.", snapshot_status);
    is_reinfoce_flag_ = false;
    std::vector<std::string> names_rh;
    auto rh = this->configuration_->read_rh_from_file(RH_SSR_OPERATE_DATA_FIRST);
    auto& rh_reinforcements = rh->reinforcement();
    for (auto iter = rh_reinforcements.begin(); iter != rh_reinforcements.end(); ++iter)
    {
        names_rh.push_back(iter->name());
    }

    if (snapshot_status == SSR_INITIAL_STATUS)
    {
        snapshot_status_ = SSR_INITIAL_STATUS;
        if (names_rh.empty())
        {
            // 需回退的加固项为空 不需要进行加固了 Reinforce Finish
            this->ProgressFinished();
            is_reinfoce_flag_ = true;
            return;
        }
        Reinforce(names_rh);

        snapshot_status_ = SSR_OTHER_STATUS;
    }
    else if (snapshot_status == SSR_LAST_REINFORCEMENT_STATUS)
    {
        snapshot_status_ = SSR_LAST_REINFORCEMENT_STATUS;
        if (names_rh.empty())
        {
            // 需回退的加固项为空 不需要进行加固了 Reinforce Finish
            this->ProgressFinished();
            is_reinfoce_flag_ = true;
            return;
        }
        Reinforce(names_rh);

        snapshot_status_ = SSR_OTHER_STATUS;
    }
    else
    {
        snapshot_status_ = SSR_OTHER_STATUS;
        is_reinfoce_flag_ = true;
    }
    //    is_reinfoce_flag_ = true;
}

std::string DBus::ActivateByActivationCode(const std::string& activation_code)
{
    KLOG_PROFILE("Activation code: %s.", activation_code.c_str());
    std::string retval = "";

    try
    {
        this->license_object_proxy_->ActivateByActivationCode(activation_code);
    }
    catch (const ::DBus::Error& e)
    {
        KLOG_WARNING("%s: %s.", e.name(), e.message());
        retval = e.message();
        throw e;
    }

    return retval;
}

void DBus::on_get_property(::DBus::InterfaceAdaptor& interface, const std::string& property, ::DBus::Variant& value)
{
    value.clear();
    if (property == "version")
    {
        value.writer().append_string(PROJECT_VERSION);
    }
    else if (property == "standard_type")
    {
        value.writer().append_uint32((uint32_t)this->configuration_->get_standard_type());
    }
    else if (property == "strategy_type")
    {
        value.writer().append_uint32((uint32_t)this->configuration_->get_strategy_type());
    }
    else if (property == "resource_monitor")
    {
        value.writer().append_uint32((uint32_t)this->configuration_->get_resource_monitor_status());
        KLOG_DEBUG("get resource monitor status");
    }
    else if (property == "time_scan")
    {
        value.writer().append_uint32((uint32_t)this->configuration_->get_time_scan());
        KLOG_DEBUG("get time scan");
    }
    else if (property == "notification_status")
    {
        value.writer().append_uint32((uint32_t)this->configuration_->get_notification_status());
        KLOG_DEBUG("get notification status");
    }
    else
    {
        KLOG_WARNING("Unknown property: %s.", property.c_str());
    }
}

void DBus::on_set_property(::DBus::InterfaceAdaptor& interface, const std::string& property, const ::DBus::Variant& value)
{
    if (property == "version")
    {
        KLOG_WARNING("Unsupport to set property 'version'.");
    }
    else if (property == "standard_type")
    {
        uint32_t standard_type = (uint32_t)value;
        this->configuration_->set_standard_type(SSRStandardType(standard_type));
    }
    else if (property == "strategy_type")
    {
        uint32_t strategy_type = (uint32_t)value;
        this->configuration_->set_strategy_type(SSRStrategyType(strategy_type));
    }
    else if (property == "resource_monitor")
    {
        uint32_t resource_monitor = (uint32_t)value;
        this->configuration_->set_resource_monitor_status(SSRResourceMonitor(resource_monitor));
    }
    else if (property == "time_scan")
    {
        uint32_t time_scan = (uint32_t)value;
        this->configuration_->set_time_scan(int(time_scan));
    }
    else if (property == "notification_status")
    {
        uint32_t notification_status = (uint32_t)value;
        this->configuration_->set_notification_status(SSRNotificationStatus(notification_status));
    }
    else
    {
        KLOG_WARNING("Unknown property: %s.", property.c_str());
    }
}

void DBus::init()
{
    this->configuration_ = Configuration::get_instance();
    this->categories_ = Categories::get_instance();
    this->plugins_ = Plugins::get_instance();
    try
    {
        this->license_manager_proxy_ = std::make_shared<LicenseManagerProxy>(this->dbus_connection_,
                                                                             LICENSE_MANAGER_OBJECT_PATH,
                                                                             LICENSE_MANAGER_DBUS_NAME);

        auto object_path = this->license_manager_proxy_->GetLicenseObject(LICENSE_OBJECT_SSR_NAME);

        this->license_object_proxy_ = std::make_shared<LicenseObjectProxy>(this->dbus_connection_,
                                                                           object_path,
                                                                           LICENSE_MANAGER_DBUS_NAME);

        this->license_object_proxy_->signal_license_changed().connect(sigc::mem_fun(this, &DBus::on_license_info_changed_cb));

        this->update_license_info();
    }
    catch (const ::DBus::Error& e)
    {
        KLOG_WARNING("%s: %s.", e.name(), e.message());
    }

    this->resource_monitor_ = new ResourceMonitor();
    KLOG_DEBUG("init ResourceMonitor.");
    this->resource_monitor_->signal_home_free_space_ratio().connect(sigc::mem_fun(this, &DBus::homeFreeSpaceRatio));
    this->resource_monitor_->signal_root_free_space_ratio().connect(sigc::mem_fun(this, &DBus::rootFreeSpaceRatio));
    this->resource_monitor_->signal_cpu_average_load_ratio().connect(sigc::mem_fun(this, &DBus::cpuAverageLoadRatio));
    this->resource_monitor_->signal_memory_remaining_ratio().connect(sigc::mem_fun(this, &DBus::memoryRemainingRatio));

    if (configuration_->get_resource_monitor_status() == SSRResourceMonitor::SSR_RESOURCE_MONITOR_OPEN)
    {
        auto timeout = Glib::MainContext::get_default()->signal_timeout();
        this->timeout_handler_ = timeout.connect(sigc::mem_fun(this, &DBus::on_resource_monitor), RESOURCEMONITORMS);
    }
}

void DBus::update_license_info()
{
    RETURN_IF_FALSE(this->license_object_proxy_);

    try
    {
        auto license_info = this->license_object_proxy_->GetLicense();
        this->license_values = StrUtils::str2json(license_info);
    }
    catch (const ::DBus::Error& e)
    {
        KLOG_WARNING("%s: %s.", e.name(), e.message());
    }
}

void DBus::on_scan_process_changed_cb(const JobResult& job_result)
{
    KLOG_PROFILE("");

    Protocol::JobResult scan_result(0, 0, 0);

    try
    {
        scan_result.process(job_result.finished_operation_num * 100.0 / job_result.sum_operation_num);
        scan_result.job_id(job_result.job_id);
        scan_result.job_state(this->scan_job_->get_state());

        int32_t item_count = 0;
        for (auto iter = job_result.running_operations.begin(); iter != job_result.running_operations.end(); ++iter)
        {
            auto operation = this->scan_job_->get_operation((*iter));

            Protocol::ReinforcementResult reinforcement_result(std::string(), 0);
            reinforcement_result.name(operation->reinforcement_name);
            reinforcement_result.state(SSRReinforcementState::SSR_REINFORCEMENT_STATE_SCANNING);
            reinforcement_result.args("");
            scan_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }

        for (auto iter = job_result.current_finished_operations.begin(); iter != job_result.current_finished_operations.end(); ++iter)
        {
            auto& operation_result = (*iter);
            auto operation = this->scan_job_->get_operation(operation_result.operation_id);
            Protocol::ReinforcementResult reinforcement_result(std::string(), 0);

            reinforcement_result.name(operation->reinforcement_name);

            SSRReinforcementState state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_UNKNOWN;
            Json::Value result_values = StrUtils::str2json(operation_result.result);
            // 如果结果为空应该时任务被取消了，如果在收到客户端的任务取消命令时操作已经在执行，结果也可能不为空，所以这里不能通过任务是否被取消的状态来判断
            if (result_values.isNull())
            {
                state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_UNSCAN;
                reinforcement_result.args("");
            }
            else if (result_values[JOB_ERROR_STR].isString())
            {
                state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_SCAN_ERROR;
                reinforcement_result.args("");
                reinforcement_result.error(result_values[JOB_ERROR_STR].asString());
            }
            else
            {
                state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_SCAN_DONE;
                reinforcement_result.args(StrUtils::json2str(result_values));
            }
            auto reinforcement = this->plugins_->get_reinforcement(operation->reinforcement_name);

            // 上一次历史操作存入rh文件
            if (!is_scan_flag_)
            {
                auto rh_reinforcement = reinforcement->get_rs();
                auto& iter_args = rh_reinforcement.arg();
                for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                {
                    if (!result_values[JOB_RETURN_VALUE][iter_arg->name()].asString().empty())
                        iter_arg->value(StrUtils::json2str(result_values[JOB_RETURN_VALUE][iter_arg->name()]));
                    KLOG_DEBUG("fix arg StrUtils::json2str(result_values[JOB_RETURN_VALUE][i]) = %s ", StrUtils::json2str(result_values[JOB_RETURN_VALUE][iter_arg->name()]).c_str());
                    KLOG_DEBUG("iter_arg : name : %s value : %s", iter_arg->name().c_str(), iter_arg->value().c_str());
                }
                this->configuration_->set_custom_rh(rh_reinforcement, RH_SSR_OPERATE_DATA_LAST);
            }

            if ((state & SSRReinforcementState::SSR_REINFORCEMENT_STATE_SCAN_DONE) != 0 &&
                reinforcement &&
                reinforcement->match_rules(result_values[JOB_RETURN_VALUE]))
            {
                state = SSRReinforcementState(state | SSRReinforcementState::SSR_REINFORCEMENT_STATE_SAFE);
            }
            else
            {
                state = SSRReinforcementState(state | SSRReinforcementState::SSR_REINFORCEMENT_STATE_UNSAFE);

                // 首次加固修改保存的历史操作文件，改为获取到的值
                if (is_frist_reinfoce_finish_)
                {
                    auto rh_frist = this->configuration_->read_rh_from_file(RH_SSR_OPERATE_DATA_FIRST);
                    //                    KLOG_DEBUG("is_frist_reinfoce_finish_ : %s", RH_SSR_OPERATE_DATA_FIRST);

                    auto& reinforcements = rh_frist->reinforcement();
                    for (auto iter = reinforcements.begin(); iter != reinforcements.end(); ++iter)
                    {
                        CONTINUE_IF_TRUE(iter->name() != reinforcement_result.name());
                        //                        KLOG_DEBUG("iter->name() = %s reinforcement_result.name = %s suscess", iter->name().c_str(), reinforcement_result.name().c_str());
                        auto& iter_args = iter->arg();
                        for (auto iter_arg = iter_args.begin(); iter_arg != iter_args.end(); ++iter_arg)
                        {
                            if (!result_values[JOB_RETURN_VALUE][iter_arg->name()].asString().empty())
                                iter_arg->value(StrUtils::json2str(result_values[JOB_RETURN_VALUE][iter_arg->name()]));
                            //                            KLOG_DEBUG("fix arg StrUtils::json2str(result_values[JOB_RETURN_VALUE][i]) = %s ", StrUtils::json2str(result_values[JOB_RETURN_VALUE][iter_arg->name()]).c_str());
                            //                            KLOG_DEBUG("iter_arg : name : %s value : %s", iter_arg->name().c_str(), iter_arg->value().c_str());
                        }
                    }
                    this->configuration_->write_rh_to_file(rh_frist, RH_SSR_OPERATE_DATA_FIRST);
                }
            }
            reinforcement_result.state(int32_t(state));
            scan_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }

        if (is_scan_flag_)
        {
            std::ostringstream ostring_stream;
            Protocol::ssr_job_result(ostring_stream, scan_result);
            this->ScanProgress(ostring_stream.str());
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }
}

void DBus::on_reinfoce_process_changed_cb(const JobResult& job_result)
{
    KLOG_PROFILE("");

    Protocol::JobResult reinforce_result(0, 0, 0);

    try
    {
        reinforce_result.process(job_result.finished_operation_num * 100.0 / job_result.sum_operation_num);
        reinforce_result.job_id(job_result.job_id);
        reinforce_result.job_state(this->reinforce_job_->get_state());

        int32_t item_count = 0;
        for (auto iter = job_result.running_operations.begin(); iter != job_result.running_operations.end(); ++iter)
        {
            auto operation = this->reinforce_job_->get_operation(*iter);
            Protocol::ReinforcementResult reinforcement_result(std::string(), 0);

            reinforcement_result.name(operation->reinforcement_name);
            reinforcement_result.state(SSRReinforcementState::SSR_REINFORCEMENT_STATE_REINFORCING);
            reinforce_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }

        for (auto iter = job_result.current_finished_operations.begin(); iter != job_result.current_finished_operations.end(); ++iter)
        {
            auto& operation_result = (*iter);
            auto operation = this->reinforce_job_->get_operation(operation_result.operation_id);
            Protocol::ReinforcementResult reinforcement_result(std::string(), 0);

            reinforcement_result.name(operation->reinforcement_name);

            SSRReinforcementState state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_UNKNOWN;
            Json::Value result_values = StrUtils::str2json(operation_result.result);

            if (result_values.isNull())
            {
                state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_UNREINFORCE;
            }
            else if (result_values[JOB_ERROR_STR].isString())
            {
                state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_REINFORCE_ERROR;
                reinforcement_result.error(result_values[JOB_ERROR_STR].asString());
            }
            else
            {
                state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_REINFORCE_DONE;
                // reinforcement_result.args(StrUtils::json2str(result_values));
            }
            reinforcement_result.state(int32_t(state));
            reinforce_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }
        if (is_reinfoce_flag_)
        {
            std::ostringstream ostring_stream;
            Protocol::ssr_job_result(ostring_stream, reinforce_result);
            this->ReinforceProgress(ostring_stream.str());
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }
}

void DBus::on_license_info_changed_cb(bool placeholder)
{
    this->update_license_info();
    this->LicenseChanged(placeholder);
}

bool DBus::on_resource_monitor()
{
    KLOG_DEBUG("on_resource_monitor.");
    if (configuration_->get_resource_monitor_status() == SSRResourceMonitor::SSR_RESOURCE_MONITOR_OPEN)
        resource_monitor_->startMonitor();
    else if (configuration_->get_resource_monitor_status() == SSRResourceMonitor::SSR_RESOURCE_MONITOR_CLOSE)
        resource_monitor_->closeMonitor();
    else
        resource_monitor_->startMonitor();
    return true;
}

void DBus::homeFreeSpaceRatio(const float space_ratio)
{
    // 家目录可用空间小于10%告警
    float homeSpa = 0.1;
    if (space_ratio >= homeSpa)
    {
        return;
    }

    KLOG_WARNING("Home free space less than 10%. The remaining space ratio is %f.", space_ratio);

    _audit_log(1101, -1, "home free space less than 10%.");

    this->HomeFreeSpaceRatioLower(std::to_string(space_ratio));
}

void DBus::rootFreeSpaceRatio(const float space_ratio)
{
    // 根目录可用空间小于10%告警
    float rootSpa = 0.1;
    if (space_ratio >= rootSpa)
    {
        return;
    }

    KLOG_WARNING("Root free space less than 10%. The remaining space ratio is %f.", space_ratio);

    _audit_log(1101, -1, "root free space less than 10%.");

    this->RootFreeSpaceRatioLower(std::to_string(space_ratio));
}

void DBus::cpuAverageLoadRatio(const float load_ratio)
{
    // cpu单核五分钟平均负载大于1告警
    float cpuLoad = 1;
    if (load_ratio < cpuLoad)
    {
        return;
    }

    KLOG_WARNING("The average load of a single core CPU exceeds 1. The average load ratio is %f.", load_ratio);

    _audit_log(1101, -1, "The average load of a single core CPU exceeds 1.");

    this->CpuAverageLoadRatioHigher(std::to_string(load_ratio));
}

void DBus::memoryRemainingRatio(const float memory_ratio)
{
    // memory ratio 小于10% 告警
    if (memory_ratio >= 0.1)
    {
        return;
    }

    KLOG_WARNING("Memory space remaining %f, below 10%", memory_ratio);

    _audit_log(1101, -1, "Memory space less than 10%");

    this->MemoryAbnormal(std::to_string(memory_ratio));
}

}  // namespace Daemon
}  // namespace KS
