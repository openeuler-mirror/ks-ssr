/**
 * @file          /ks-ssr-manager/src/daemon/dbus.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/dbus.h"
#include <json/json.h>
#include <kylin-license/license-i.h>
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
#define JOB_ERROR_STR "error"
#define JOB_RETURN_VALUE "return_value"

std::shared_ptr<SSRReinforcementInterface> reinforcement_interface_;
std::string param_str_ = "";

DBus::DBus(::DBus::Connection& connection) : ::DBus::ObjectAdaptor(connection, SSR_DBUS_OBJECT_PATH),
                                             dbus_connection_(connection)
{
}

DBus::~DBus()
{
}

DBus* DBus::instance_ = NULL;
void DBus::global_init(::DBus::Connection& connection)
{
    instance_ = new DBus(connection);
    instance_->init();
}

void DBus::SetStandardType(const uint32_t& standard_type)
{
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
    KLOG_PROFILE("encoded standard: %s.", encoded_standard.c_str());

    SSRErrorCode error_code = SSRErrorCode::SUCCESS;
    if (!this->configuration_->set_custom_rs(encoded_standard, error_code))
    {
        THROW_DBUSCXX_ERROR(error_code);
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
    KLOG_PROFILE("");

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
    KLOG_PROFILE("");
    this->configuration_->del_custom_ra(name);
}

static std::string ReinforcementInterfaceScanFun()
{
    Json::Value retval;
    std::string args;
    std::string error;
    if (reinforcement_interface_->get(args, error))
    {
        retval[JOB_RETURN_VALUE] = StrUtils::str2json(args);
    }
    else
    {
        retval[JOB_ERROR_STR] = error;
    }
    return StrUtils::json2str(retval);
}

int64_t DBus::Scan(const std::vector<std::string>& names)
{
    KLOG_PROFILE("range: %s.", StrUtils::join(names, " ").c_str());

    // 已经在扫描则返回错误
    if (this->scan_job_ && this->scan_job_->get_state() == SSRJobState::SSR_JOB_STATE_RUNNING)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SCAN_IS_RUNNING);
    }

    try
    {
        this->scan_job_ = Job::create();

        for (auto iter = names.begin(); iter != names.end(); ++iter)
        {
            auto& name = (*iter);
            auto reinforcement = this->plugins_->get_reinforcement(name);

            if (!reinforcement)
            {
                THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND, name);
            }

            reinforcement_interface_ = this->plugins_->get_reinfocement_interface(reinforcement->get_plugin_name(),
                                                                                      reinforcement->get_name());
            if (!reinforcement_interface_)
            {
                THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND);
            }

            std::function<std::string(void)> func = ReinforcementInterfaceScanFun;
            this->scan_job_->add_operation(reinforcement->get_plugin_name(), reinforcement->get_name(), func);

            // 重新扫描时需要清理加固项的安全状态
            // reinforcement->state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_UNKNOWN;
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SCAN_RANGE_INVALID);
    }

    this->scan_job_->signal_process_changed().connect(sigc::mem_fun(this, &DBus::on_scan_process_changed_cb));

    if (!this->scan_job_->run_async())
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SCAN_ALL_JOB_FAILED);
    }

    return this->scan_job_->get_id();
}

static std::string ReinforcementInterfaceReinforceFun()
{
    std::string error;
    Json::Value retval;
    if (!reinforcement_interface_->set(param_str_, error))
    {
        retval[JOB_ERROR_STR] = error;
    }
    else
    {
    // 设置为空字符串，这里主要是为了区分加固成功和取消加固两种状态，后续可能会调整改逻辑
    retval[JOB_RETURN_VALUE] = std::string();
    }
    return StrUtils::json2str(retval);
}

int64_t DBus::Reinforce(const std::vector<std::string>& names)
{
    KLOG_PROFILE("range: %s.", StrUtils::join(names, " ").c_str());

    // 未授权不允许加固
    if (this->license_values.isNull() ||
        this->license_values[LICENSE_JK_ACTIVATION_STATUS].isNull() ||
        this->license_values[LICENSE_JK_ACTIVATION_STATUS].asInt() != LicenseActivationStatus::LAS_ACTIVATED)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_SOFTWARE_UNACTIVATED);
    }

    // 已经在加固则返回错误
    if (this->reinforce_job_ && this->reinforce_job_->get_state() == SSRJobState::SSR_JOB_STATE_RUNNING)
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_REINFORCE_IS_RUNNING);
    }

    try
    {
        this->reinforce_job_ = Job::create();

        for (auto iter = names.begin(); iter != names.end(); ++iter)
        {
            auto& name = (*iter);
            auto reinforcement = this->plugins_->get_reinforcement(name);

            if (!reinforcement)
            {
                THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND, name);
            }

            reinforcement_interface_ = this->plugins_->get_reinfocement_interface(reinforcement->get_plugin_name(),
                                                                                      reinforcement->get_name());
            if (!reinforcement_interface_)
            {
                THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND);
            }

            Json::Value param;
            auto& args = reinforcement->get_rs().arg();
            for (auto arg_iter = args.begin(); arg_iter != args.end(); ++arg_iter)
            {
                param[arg_iter->name()] = StrUtils::str2json(arg_iter->value());
            }
            param_str_ = StrUtils::json2str(param);
            std::function<std::string(void)> func = ReinforcementInterfaceReinforceFun;
            this->reinforce_job_->add_operation(reinforcement->get_plugin_name(), reinforcement->get_name(), func);
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_DAEMON_REINFORCE_RANGE_INVALID);
    }

    this->reinforce_job_->signal_process_changed().connect(sigc::mem_fun(this, &DBus::on_reinfoce_process_changed_cb));

    if (!this->reinforce_job_->run_async())
    {
        THROW_DBUSCXX_ERROR(SSRErrorCode::ERROR_CORE_REINFORCE_JOB_FAILED);
    }

    return this->reinforce_job_->get_id();
}

void DBus::Cancel(const int64_t& job_id)
{
    KLOG_PROFILE("job id: %d.", job_id);

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

void DBus::ActivateByActivationCode(const std::string& activation_code)
{
    KLOG_PROFILE("Activation code: %s.", activation_code.c_str());

    try
    {
        this->license_object_proxy_->ActivateByActivationCode(activation_code);
    }
    catch (const ::DBus::Error& e)
    {
        KLOG_WARNING("%s: %s.", e.name(), e.message());
        throw e;
    }
}

void DBus::on_get_property(::DBus::InterfaceAdaptor& interface, const std::string& property, ::DBus::Variant& value)
{
    if (property == "version")
    {
        value.writer().append_string(PROJECT_VERSION);
    }
    else if (property == "standard_type")
    {
        value.writer().append_uint32(this->configuration_->get_standard_type());
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
        uint32_t standard_type = value;
        this->configuration_->set_standard_type(SSRStandardType(standard_type));
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
            }
            else if (result_values[JOB_ERROR_STR].isString())
            {
                state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_SCAN_ERROR;
                reinforcement_result.error(result_values[JOB_ERROR_STR].asString());
            }
            else
            {
                state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_SCAN_DONE;
            }

            auto reinforcement = this->plugins_->get_reinforcement(operation->reinforcement_name);

            if ((state & SSRReinforcementState::SSR_REINFORCEMENT_STATE_SCAN_DONE) != 0 &&
                reinforcement &&
                reinforcement->match_rules(result_values[JOB_RETURN_VALUE]))
            {
                state = SSRReinforcementState(state | SSRReinforcementState::SSR_REINFORCEMENT_STATE_SAFE);
            }
            else
            {
                state = SSRReinforcementState(state | SSRReinforcementState::SSR_REINFORCEMENT_STATE_UNSAFE);
            }

            reinforcement_result.state(int32_t(state));
            scan_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }

        std::ostringstream ostring_stream;
        Protocol::ssr_job_result(ostring_stream, scan_result);
        this->ScanProgress(ostring_stream.str());
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
            }
            reinforcement_result.state(int32_t(state));
            reinforce_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }

        std::ostringstream ostring_stream;
        Protocol::ssr_job_result(ostring_stream, reinforce_result);
        this->ReinforceProgress(ostring_stream.str());
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

}  // namespace Daemon
}  // namespace KS
