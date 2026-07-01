/**
 * @file          /kiran-sse-manager/src/daemon/sse-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/sse-manager.h"
#include <json/json.h>
#include "lib/base/base.h"
#include "lib/core/sse-categories.h"
#include "lib/core/sse-configuration.h"
#include "lib/core/sse-plugins.h"
#include "sse-i.h"

namespace Kiran
{
SSEManager::SSEManager() : dbus_connect_id_(0),
                           object_register_id_(0)
{
}

SSEManager::~SSEManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

SSEManager* SSEManager::instance_ = nullptr;
void SSEManager::global_init()
{
    instance_ = new SSEManager();
    instance_->init();
}

void SSEManager::SetStandardType(guint32 standard_type, MethodInvocation& invocation)
{
    KLOG_PROFILE("standard type: %d.", standard_type);

    if (standard_type >= SSEStandardType::SEE_STANDARD_TYPE_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_STANDARD_TYPE_INVALID);
    }

    if (standard_type == this->configuration_->get_standard_type())
    {
        invocation.ret();
    }

    if (!this->standard_type_set(standard_type))
    {
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_SET_STANDARD_TYPE_FAILED);
    }

    invocation.ret();
}

void SSEManager::ImportCustomRS(const Glib::ustring& encoded_standard, MethodInvocation& invocation)
{
    KLOG_PROFILE("encoded standard: %s.", encoded_standard.c_str());

    SSEErrorCode error_code = SSEErrorCode::SUCCESS;
    if (!this->configuration_->set_custom_rs(encoded_standard, error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    invocation.ret();
}

void SSEManager::GetCategories(MethodInvocation& invocation)
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
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_CONVERT_CATEGORIES2JSON_FAILED);
    }
}

/*void SSEManager::GetPlugins(MethodInvocation& invocation)
{
    Json::Value values;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "";
    auto plugins = this->plugins_->get_plugins();

    try
    {
        values["item_count"] = plugins.size();
        for (uint32_t i = 0; i < plugins.size(); ++i)
        {
            auto plugin = plugins[i];
            const auto& plugin_info = plugin->get_plugin_info();
            values["items"][i]["name"] = plugin_info.name;
        }

        auto result = Json::writeString(wbuilder, values);
        invocation.ret(result);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DEAMON_CONVERT_CATEGORIES2JSON_FAILED);
    }
}*/

void SSEManager::GetReinforcements(MethodInvocation& invocation)
{
    KLOG_PROFILE("");

    Json::Value result_values;
    Json::StreamWriterBuilder wbuilder;
    std::string error;

    wbuilder["indentation"] = "";

    try
    {
        auto reinforcements = this->plugins_->get_used_reinforcements();

        for (uint32_t i = 0; i < reinforcements.size(); ++i)
        {
            auto reinforcement = reinforcements[i];

            result_values["items"][i]["name"] = reinforcement->name;
            result_values["items"][i]["category_name"] = reinforcement->category_name;
            result_values["items"][i]["label"] = reinforcement->label;
            if (!reinforcement->custom_args.isNull())
            {
                result_values["items"][i]["args"] = reinforcement->custom_args;
            }
            else
            {
                result_values["items"][i]["args"] = reinforcement->default_args;
            }
            result_values["items"][i]["layout"] = reinforcement->layout;
        }
        result_values["item_count"] = reinforcements.size();
        auto result = Json::writeString(wbuilder, result_values);
        invocation.ret(result);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_RS_CONTENT_INVALID);
    }
}

void SSEManager::SetReinforcementArgs(const Glib::ustring& name,
                                      const Glib::ustring& custom_args,
                                      MethodInvocation& invocation)
{
    if (!this->plugins_->set_reinforcement_arguments(name, custom_args))
    {
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_SET_REINFORCEMENT_ARGS_FAILED);
    }
    invocation.ret();
}

void SSEManager::Scan(const Glib::ustring& range, MethodInvocation& invocation)
{
    KLOG_PROFILE("range: %s.", range.c_str());

    // 已经在扫描则返回错误
    if (this->scan_job_ && this->scan_job_->get_state() == SSEJobState::SSE_JOB_STATE_RUNNING)
    {
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_SCAN_IS_RUNNING);
    }

    try
    {
        this->scan_job_ = SSEJob::create();

        auto range_json = StrUtils::str2json(range);
        for (int32_t i = 0; i < (int32_t)range_json["items"].size(); ++i)
        {
            auto name = range_json["items"][i]["name"].asString();
            auto reinforcement = this->plugins_->get_used_reinforcement(name);

            if (!reinforcement)
            {
                DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_SCAN_REINFORCEMENT_NOTFOUND, name);
            }

            auto plugin = this->plugins_->get_plugin(reinforcement->plugin_name);
            if (!plugin)
            {
                KLOG_WARNING("Plugin '%s' of the reinforcement '%s' is not found.",
                             reinforcement->plugin_name.c_str(),
                             reinforcement->name.c_str());
                DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND);
            }
            auto interface = plugin->get_loader()->get_interface();

            Json::Value param;
            param["head"]["id"] = int32_t(SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_SC_MATCH_RS_REQ);
            param["body"]["name"] = reinforcement->name;
            param["body"]["rules"] = reinforcement->rules;
            auto param_str = StrUtils::json2str(param);

            this->scan_job_->add_operation(reinforcement->plugin_name,
                                           reinforcement->name,
                                           [interface, param_str]() -> std::string {
                                               return interface->execute(param_str);
                                           });

            // 重新扫描时需要清理加固项的安全状态
            // reinforcement->state = SSEReinforcementState::SSE_REINFORCEMENT_STATE_UNKNOWN;
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_SCAN_RANGE_INVALID);
    }

    this->scan_job_->signal_process_changed().connect(sigc::mem_fun(this, &SSEManager::on_scan_process_changed_cb));

    if (!this->scan_job_->run_async())
    {
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_SCAN_ALL_JOB_FAILED);
    }
    else
    {
        invocation.ret(this->scan_job_->get_id());
    }
}

void SSEManager::Reinforce(const Glib::ustring& reinforcements, MethodInvocation& invocation)
{
    // 已经在加固则返回错误
    if (this->reinforce_job_ && this->reinforce_job_->get_state() == SSEJobState::SSE_JOB_STATE_RUNNING)
    {
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_REINFORCE_IS_RUNNING);
    }

    try
    {
        Json::Value reinforcenments_values = StrUtils::str2json(reinforcements);
        this->reinforce_job_ = SSEJob::create();

        for (uint32_t i = 0; i < reinforcenments_values["items"].size(); ++i)
        {
            auto name = reinforcenments_values["items"][i]["name"].asString();
            auto reinforcement = this->plugins_->get_used_reinforcement(name);

            if (!reinforcement)
            {
                DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_SCAN_REINFORCEMENT_NOTFOUND_2, name);
            }

            auto plugin = this->plugins_->get_plugin(reinforcement->plugin_name);
            if (!plugin)
            {
                KLOG_WARNING("Plugin '%s' of the reinforcement '%s' is not found.",
                             reinforcement->plugin_name.c_str(),
                             reinforcement->name.c_str());
                DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND_2);
            }
            auto interface = plugin->get_loader()->get_interface();

            Json::Value param;
            param["head"]["id"] = SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_REINFORCE_REQ;
            param["body"]["name"] = reinforcement->name;
            if (!reinforcement->custom_args.isNull())
            {
                param["body"]["args"] = reinforcement->custom_args;
            }
            else
            {
                param["body"]["args"] = reinforcement->default_args;
            }

            auto param_str = StrUtils::json2str(param);
            this->reinforce_job_->add_operation(reinforcement->plugin_name,
                                                reinforcement->name,
                                                [interface, param_str]() -> std::string {
                                                    return interface->execute(param_str);
                                                });
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_DAEMON_REINFORCE_RANGE_INVALID);
    }

    this->reinforce_job_->signal_process_changed().connect(sigc::mem_fun(this, &SSEManager::on_reinfoce_process_changed_cb));

    if (!this->reinforce_job_->run_async())
    {
        DBUS_ERROR_REPLY_AND_RET(SSEErrorCode::ERROR_CORE_REINFORCE_JOB_FAILED);
    }
    else
    {
        invocation.ret(this->reinforce_job_->get_id());
    }
}

void SSEManager::Cancel(gint64 job_id, MethodInvocation& invocation)
{
    KLOG_PROFILE("job id: %d.", job_id);

    SSEErrorCode error_code = SSEErrorCode::SUCCESS;

    if (this->scan_job_ &&
        job_id == this->scan_job_->get_id() &&
        this->scan_job_->get_state() == SSEJobState::SSE_JOB_STATE_RUNNING)
    {
        if (!this->scan_job_->cancel())
        {
            error_code = SSEErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_1;
        }
    }
    else if (this->reinforce_job_ &&
             job_id == this->reinforce_job_->get_id() &&
             this->reinforce_job_->get_state() == SSEJobState::SSE_JOB_STATE_RUNNING)
    {
        if (!this->reinforce_job_->cancel())
        {
            error_code = SSEErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_2;
        }
    }
    else
    {
        error_code = SSEErrorCode::ERROR_DAEMON_CANCEL_NOTFOUND_JOB;
    }

    if (error_code != SSEErrorCode::SUCCESS)
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    invocation.ret();
}

bool SSEManager::standard_type_setHandler(guint32 value)
{
    return this->configuration_->set_standard_type(SSEStandardType(value));
}

guint32 SSEManager::standard_type_get()
{
    return this->configuration_->get_standard_type();
}

void SSEManager::init()
{
    this->configuration_ = SSEConfiguration::get_instance();
    this->categories_ = SSECategories::get_instance();
    this->plugins_ = SSEPlugins::get_instance();

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 SSE_DBUS_NAME,
                                                 sigc::mem_fun(this, &SSEManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &SSEManager::on_name_acquired),
                                                 sigc::mem_fun(this, &SSEManager::on_name_lost));
}

void SSEManager::on_scan_process_changed_cb(const SSEJobResult& job_result)
{
    Json::Value scan_result;

    try
    {
        scan_result["process"] = double(job_result.finished_operation_num * 100.0 / job_result.sum_operation_num);
        scan_result["job_id"] = job_result.job_id;
        scan_result["job_state"] = this->scan_job_->get_state();

        int32_t item_count = 0;
        for (auto operation_id : job_result.running_operations)
        {
            auto operation = this->scan_job_->get_operation(operation_id);
            scan_result["items"][item_count]["name"] = operation->reforcement_name;
            scan_result["items"][item_count]["state"] = SSEReinforcementState::SSE_REINFORCEMENT_STATE_SCANNING;
            ++item_count;
        }

        for (const auto& operation_result : job_result.current_finished_operations)
        {
            auto operation = this->scan_job_->get_operation(operation_result.operation_id);
            scan_result["items"][item_count]["name"] = operation->reforcement_name;

            SSEReinforcementState state = SSEReinforcementState::SSE_REINFORCEMENT_STATE_UNKNOWN;
            Json::Value result_values = StrUtils::str2json(operation_result.result);
            // 如果结果为空应该时任务被取消了，如果在收到客户端的任务取消命令时操作已经在执行，结果也可能不为空，所以这里不能通过任务是否被取消的状态来判断
            if (result_values.isNull())
            {
                state = SSEReinforcementState::SSE_REINFORCEMENT_STATE_UNSCAN;
            }
            else
            {
                state = SSEReinforcementState::SSE_REINFORCEMENT_STATE_SCAN_DONE;
            }

            if (result_values.isMember("body") && result_values["body"]["match"].asBool())
            {
                state = SSEReinforcementState(state | SSEReinforcementState::SSE_REINFORCEMENT_STATE_SAFE);
            }
            else
            {
                state = SSEReinforcementState(state | SSEReinforcementState::SSE_REINFORCEMENT_STATE_UNSAFE);
            }
            scan_result["items"][item_count]["state"] = int32_t(state);
            ++item_count;
        }

        scan_result["item_count"] = item_count;
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }

    auto result = StrUtils::json2str(scan_result);
    this->ScanProgress_signal.emit(result);
}

void SSEManager::on_reinfoce_process_changed_cb(const SSEJobResult& job_result)
{
    Json::Value reinforce_result;

    try
    {
        reinforce_result["process"] = double(job_result.finished_operation_num * 100.0 / job_result.sum_operation_num);
        reinforce_result["job_id"] = job_result.job_id;
        reinforce_result["job_state"] = this->reinforce_job_->get_state();

        int32_t item_count = 0;
        for (auto operation_id : job_result.running_operations)
        {
            auto operation = this->reinforce_job_->get_operation(operation_id);
            reinforce_result["items"][item_count]["name"] = operation->reforcement_name;
            reinforce_result["items"][item_count]["state"] = SSEReinforcementState::SSE_REINFORCEMENT_STATE_REINFORCING;
            ++item_count;
        }

        for (const auto& operation_result : job_result.current_finished_operations)
        {
            auto operation = this->reinforce_job_->get_operation(operation_result.operation_id);
            reinforce_result["items"][item_count]["name"] = operation->reforcement_name;

            SSEReinforcementState state = SSEReinforcementState::SSE_REINFORCEMENT_STATE_UNKNOWN;
            Json::Value result_values = StrUtils::str2json(operation_result.result);

            if (result_values.isNull())
            {
                state = SSEReinforcementState::SSE_REINFORCEMENT_STATE_UNREINFORCE;
            }
            else
            {
                state = SSEReinforcementState::SSE_REINFORCEMENT_STATE_REINFORCE_DONE;
            }

            if (result_values["head"].isMember("error_code"))
            {
                SSEErrorCode error_code = SSEErrorCode(result_values["head"]["error_code"].asInt());
                if (error_code != SSEErrorCode::SUCCESS)
                {
                    reinforce_result["items"][item_count]["error"] = CC_ERROR2STR(error_code);
                }
            }
            reinforce_result["items"][item_count]["state"] = int32_t(state);
            ++item_count;
        }

        reinforce_result["item_count"] = item_count;
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }

    auto result = StrUtils::json2str(reinforce_result);
    this->ReinforceProgress_signal.emit(result);
}

void SSEManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        KLOG_WARNING("Failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, SSE_DBUS_OBJECT_PATH);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Register object_path %s fail: %s.", SSE_DBUS_OBJECT_PATH, e.what().c_str());
    }
}

void SSEManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_DEBUG("Success to register dbus name: %s", name.c_str());
}

void SSEManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_WARNING("Failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran