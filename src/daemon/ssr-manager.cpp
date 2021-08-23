/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/ssr-manager.h"
#include <json/json.h>
#include <iostream>
#include "lib/base/base.h"
#include "src/daemon/ssr-categories.h"
#include "src/daemon/ssr-configuration.h"
#include "src/daemon/ssr-plugins.h"
#include "src/daemon/ssr-protocol.hxx"
#include "src/daemon/ssr-utils.h"

namespace Kiran
{
#define JOB_ERROR_CODE "error_code"
#define JOB_RETURN_VALUE "return_value"

SSRManager::SSRManager() : dbus_connect_id_(0),
                           object_register_id_(0)
{
}

SSRManager::~SSRManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

SSRManager* SSRManager::instance_ = nullptr;
void SSRManager::global_init()
{
    instance_ = new SSRManager();
    instance_->init();
}

void SSRManager::SetStandardType(guint32 standard_type, MethodInvocation& invocation)
{
    KLOG_PROFILE("standard type: %d.", standard_type);

    if (standard_type >= SSRStandardType::SSR_STANDARD_TYPE_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_STANDARD_TYPE_INVALID);
    }

    if (standard_type == this->configuration_->get_standard_type())
    {
        invocation.ret();
    }

    if (!this->standard_type_set(standard_type))
    {
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_SET_STANDARD_TYPE_FAILED);
    }

    invocation.ret();
}

void SSRManager::ImportCustomRS(const Glib::ustring& encoded_standard, MethodInvocation& invocation)
{
    KLOG_PROFILE("encoded standard: %s.", encoded_standard.c_str());

    SSRErrorCode error_code = SSRErrorCode::SUCCESS;
    if (!this->configuration_->set_custom_rs(encoded_standard, error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    invocation.ret();
}

void SSRManager::GetCategories(MethodInvocation& invocation)
{
    KLOG_PROFILE("");

    Json::Value values;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "";

    auto categories = this->categories_->get_categories();

    try
    {
        values["item_count"] = categories.size();
        for (uint32_t i = 0; i < categories.size(); ++i)
        {
            auto category = categories[i];

            values["items"][i]["name"] = category->name;
            values["items"][i]["label"] = category->label;
            values["items"][i]["icon_name"] = category->icon_name;
        }

        auto result = Json::writeString(wbuilder, values);
        invocation.ret(result);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_CONVERT_CATEGORIES2JSON_FAILED);
    }
}

void SSRManager::GetRS(MethodInvocation& invocation)
{
    std::ostringstream ostring_stream;
    auto rs = this->configuration_->get_rs();

    if (!rs)
    {
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_GET_RS_FAILED);
    }

    try
    {
        Protocol::ssr_rs(ostring_stream, *rs.get());
        invocation.ret(ostring_stream.str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_GET_RS_FAILED);
    }
}

void SSRManager::GetReinforcements(MethodInvocation& invocation)
{
    KLOG_PROFILE("");

    Protocol::Reinforcements protocol_reinforcements;

    for (auto reinforcement : this->plugins_->get_reinforcements())
    {
        auto& rs_reinforcement = reinforcement->get_rs();
        protocol_reinforcements.reinforcement().push_back(rs_reinforcement);
    }

    try
    {
        std::ostringstream ostring_stream;
        Protocol::ssr_reinforcements(ostring_stream, protocol_reinforcements);
        invocation.ret(ostring_stream.str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENT_FAILED);
    }
}

void SSRManager::GetReinforcement(const Glib::ustring& name, MethodInvocation& invocation)
{
    KLOG_PROFILE("");

    auto reinforcement = this->plugins_->get_reinforcement(name);
    if (!reinforcement)
    {
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND);
    }
    auto& rs_reinforcement = reinforcement->get_rs();

    try
    {
        std::ostringstream ostring_stream;
        Protocol::ssr_reinforcement(ostring_stream, rs_reinforcement);
        invocation.ret(ostring_stream.str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENT_FAILED);
    }
}

void SSRManager::SetReinforcement(const Glib::ustring& reinforcement_xml, MethodInvocation& invocation)
{
    KLOG_PROFILE("");

    try
    {
        std::istringstream istring_stream(reinforcement_xml);
        auto rs_reinforcement = Protocol::ssr_reinforcement(istring_stream, xml_schema::Flags::dont_validate);
        if (!this->configuration_->set_custom_ra(*rs_reinforcement.get()))
        {
            DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_SET_REINFORCEMENT_FAILED);
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_SET_REINFORCEMENT_FAILED);
    }
    invocation.ret();
}

void SSRManager::Scan(const std::vector<Glib::ustring>& names, MethodInvocation& invocation)
{
    KLOG_PROFILE("range: %s.", StrUtils::join(names, " ").c_str());

    // 已经在扫描则返回错误
    if (this->scan_job_ && this->scan_job_->get_state() == SSRJobState::SSR_JOB_STATE_RUNNING)
    {
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_SCAN_IS_RUNNING);
    }

    try
    {
        this->scan_job_ = SSRJob::create();

        for (auto& name : names)
        {
            auto reinforcement = this->plugins_->get_reinforcement(name);

            if (!reinforcement)
            {
                DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND, name);
            }

            auto reinforcement_interface = this->plugins_->get_reinfocement_interface(reinforcement->get_plugin_name(),
                                                                                      reinforcement->get_name());
            if (!reinforcement_interface)
            {
                DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND);
            }

            this->scan_job_->add_operation(reinforcement->get_plugin_name(),
                                           reinforcement->get_name(),
                                           [reinforcement_interface]() -> std::string {
                                               Json::Value retval;
                                               std::string args;
                                               SSRErrorCode error_code = SSRErrorCode::SUCCESS;
                                               if (reinforcement_interface->get(args, error_code))
                                               {
                                                   retval[JOB_RETURN_VALUE] = StrUtils::str2json(args);
                                               }
                                               retval[JOB_ERROR_CODE] = int32_t(error_code);
                                               return StrUtils::json2str(retval);
                                           });

            // 重新扫描时需要清理加固项的安全状态
            // reinforcement->state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_UNKNOWN;
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_SCAN_RANGE_INVALID);
    }

    this->scan_job_->signal_process_changed().connect(sigc::mem_fun(this, &SSRManager::on_scan_process_changed_cb));

    if (!this->scan_job_->run_async())
    {
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_SCAN_ALL_JOB_FAILED);
    }
    else
    {
        invocation.ret(this->scan_job_->get_id());
    }
}

void SSRManager::Reinforce(const std::vector<Glib::ustring>& names, MethodInvocation& invocation)
{
    KLOG_PROFILE("range: %s.", StrUtils::join(names, " ").c_str());

    // 已经在加固则返回错误
    if (this->reinforce_job_ && this->reinforce_job_->get_state() == SSRJobState::SSR_JOB_STATE_RUNNING)
    {
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_REINFORCE_IS_RUNNING);
    }

    try
    {
        this->reinforce_job_ = SSRJob::create();

        for (auto& name : names)
        {
            auto reinforcement = this->plugins_->get_reinforcement(name);

            if (!reinforcement)
            {
                DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND, name);
            }

            auto reinforcement_interface = this->plugins_->get_reinfocement_interface(reinforcement->get_plugin_name(),
                                                                                      reinforcement->get_name());
            if (!reinforcement_interface)
            {
                DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND);
            }

            Json::Value param;
            for (const auto& arg : reinforcement->get_rs().arg())
            {
                param[arg.name()] = StrUtils::str2json(arg.value());
            }
            auto param_str = StrUtils::json2str(param);
            this->reinforce_job_->add_operation(reinforcement->get_plugin_name(),
                                                reinforcement->get_name(),
                                                [reinforcement_interface, param_str]() -> std::string {
                                                    SSRErrorCode error_code = SSRErrorCode::SUCCESS;
                                                    reinforcement_interface->set(param_str, error_code);
                                                    Json::Value retval;
                                                    retval[JOB_ERROR_CODE] = int32_t(error_code);
                                                    return StrUtils::json2str(retval);
                                                });
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_DAEMON_REINFORCE_RANGE_INVALID);
    }

    this->reinforce_job_->signal_process_changed().connect(sigc::mem_fun(this, &SSRManager::on_reinfoce_process_changed_cb));

    if (!this->reinforce_job_->run_async())
    {
        DBUS_ERROR_REPLY_AND_RET(SSRErrorCode::ERROR_CORE_REINFORCE_JOB_FAILED);
    }
    else
    {
        invocation.ret(this->reinforce_job_->get_id());
    }
}

void SSRManager::Cancel(gint64 job_id, MethodInvocation& invocation)
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
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    invocation.ret();
}

bool SSRManager::standard_type_setHandler(guint32 value)
{
    return this->configuration_->set_standard_type(SSRStandardType(value));
}

guint32 SSRManager::standard_type_get()
{
    return this->configuration_->get_standard_type();
}

void SSRManager::init()
{
    this->configuration_ = SSRConfiguration::get_instance();
    this->categories_ = SSRCategories::get_instance();
    this->plugins_ = SSRPlugins::get_instance();

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 SSR_DBUS_NAME,
                                                 sigc::mem_fun(this, &SSRManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &SSRManager::on_name_acquired),
                                                 sigc::mem_fun(this, &SSRManager::on_name_lost));
}

Json::Value SSRManager::get_reinforcement_json(const std::string& name)
{
    Json::Value retval;

    try
    {
        auto reinforcement = this->plugins_->get_reinforcement(name);

        retval[SSR_JSON_BODY_REINFORCEMENT_NAME] = reinforcement->get_name();
        retval[SSR_JSON_BODY_REINFORCEMENT_CATEGORY_NAME] = reinforcement->get_category_name();
        retval[SSR_JSON_BODY_REINFORCEMENT_LABEL] = reinforcement->get_label();
        for (const auto& arg : reinforcement->get_rs().arg())
        {
            Json::Value arg_value;
            arg_value[SSR_JSON_BODY_REINFORCEMENT_ARG_NAME] = arg.name();
            arg_value[SSR_JSON_BODY_REINFORCEMENT_ARG_VALUE] = arg.value();
            if (arg.layout().present())
            {
                arg_value[SSR_JSON_BODY_REINFORCEMENT_ARG_LAYOUT][SSR_JSON_BODY_REINFORCEMENT_ARG_LAYOUT_TYPE] = arg.layout().get().widget_type();
                arg_value[SSR_JSON_BODY_REINFORCEMENT_ARG_LAYOUT][SSR_JSON_BODY_REINFORCEMENT_ARG_LAYOUT_LABEL] = SSRUtils::get_xsd_local_value(arg.layout().get().label());
            }
            retval[SSR_JSON_BODY_REINFORCEMENT_ARGS].append(std::move(arg_value));
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        return Json::Value();
    }
    return retval;
}

void SSRManager::on_scan_process_changed_cb(const SSRJobResult& job_result)
{
    KLOG_PROFILE("");

    Protocol::JobResult scan_result(0, 0, 0);

    try
    {
        scan_result.process(job_result.finished_operation_num * 100.0 / job_result.sum_operation_num);
        scan_result.job_id(job_result.job_id);
        scan_result.job_state(this->scan_job_->get_state());

        int32_t item_count = 0;
        for (auto operation_id : job_result.running_operations)
        {
            auto operation = this->scan_job_->get_operation(operation_id);

            Protocol::ReinforcementResult reinforcement_result(std::string(), 0);
            reinforcement_result.name(operation->reinforcement_name);
            reinforcement_result.state(SSRReinforcementState::SSR_REINFORCEMENT_STATE_SCANNING);
            scan_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }

        for (const auto& operation_result : job_result.current_finished_operations)
        {
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
            else
            {
                state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_SCAN_DONE;
            }

            auto reinforcement = this->plugins_->get_reinforcement(operation->reinforcement_name);

            if (result_values[JOB_ERROR_CODE].isInt() &&
                result_values[JOB_ERROR_CODE].asInt() == int32_t(SSRErrorCode::SUCCESS) &&
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
        this->ScanProgress_signal.emit(ostring_stream.str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }
}

void SSRManager::on_reinfoce_process_changed_cb(const SSRJobResult& job_result)
{
    KLOG_PROFILE("");

    Protocol::JobResult reinforce_result(0, 0, 0);

    try
    {
        reinforce_result.process(job_result.finished_operation_num * 100.0 / job_result.sum_operation_num);
        reinforce_result.job_id(job_result.job_id);
        reinforce_result.job_state(this->reinforce_job_->get_state());

        int32_t item_count = 0;
        for (auto operation_id : job_result.running_operations)
        {
            auto operation = this->reinforce_job_->get_operation(operation_id);
            Protocol::ReinforcementResult reinforcement_result(std::string(), 0);

            reinforcement_result.name(operation->reinforcement_name);
            reinforcement_result.state(SSRReinforcementState::SSR_REINFORCEMENT_STATE_REINFORCING);
            reinforce_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }

        for (const auto& operation_result : job_result.current_finished_operations)
        {
            auto operation = this->reinforce_job_->get_operation(operation_result.operation_id);
            Protocol::ReinforcementResult reinforcement_result(std::string(), 0);

            reinforcement_result.name(operation->reinforcement_name);

            SSRReinforcementState state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_UNKNOWN;
            Json::Value result_values = StrUtils::str2json(operation_result.result);

            if (result_values.isNull())
            {
                state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_UNREINFORCE;
            }
            else
            {
                state = SSRReinforcementState::SSR_REINFORCEMENT_STATE_REINFORCE_DONE;
            }

            if (result_values[JOB_ERROR_CODE].isInt() &&
                result_values[JOB_ERROR_CODE].asInt() != int32_t(SSRErrorCode::SUCCESS))
            {
                SSRErrorCode error_code = SSRErrorCode(result_values[JOB_ERROR_CODE].asInt());
                if (error_code != SSRErrorCode::SUCCESS)
                {
                    reinforcement_result.error(CC_ERROR2STR(error_code));
                }
            }
            reinforcement_result.state(int32_t(state));
            reinforce_result.reinforcement().push_back(std::move(reinforcement_result));
            ++item_count;
        }

        std::ostringstream ostring_stream;
        Protocol::ssr_job_result(ostring_stream, reinforce_result);
        this->ReinforceProgress_signal.emit(ostring_stream.str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }
}

void SSRManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        KLOG_WARNING("Failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, SSR_DBUS_OBJECT_PATH);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Register object_path %s fail: %s.", SSR_DBUS_OBJECT_PATH, e.what().c_str());
    }
}

void SSRManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_DEBUG("Success to register dbus name: %s", name.c_str());
}

void SSRManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_WARNING("Failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran