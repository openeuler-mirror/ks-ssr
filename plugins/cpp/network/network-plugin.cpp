/**
 * @file          /kiran-sse-manager/plugins/cpp/network/network-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "plugins/cpp/network/network-plugin.h"
#include <json/json.h>
#include "lib/base/base.h"
#include "plugins/cpp/network/network-reinforcement-manager.h"

namespace Kiran
{
PLUGIN_EXPORT_FUNC_DEF(SSEPluginNetwork);

void SSEPluginNetwork::activate()
{
}

void SSEPluginNetwork::deactivate()
{
}

std::string SSEPluginNetwork::execute(const std::string& in_json)
{
    auto in_values = StrUtils::str2json(in_json);
    Json::Value out_values;
    SSEErrorCode error_code = SSEErrorCode::SUCCESS;

    try
    {
        auto request_id = in_values[SSE_JSON_HEAD][SSE_JSON_HEAD_PROTOCOL_ID].asInt();

        KLOG_DEBUG("request id: %d.", request_id);

        out_values[SSE_JSON_HEAD][SSE_JSON_HEAD_PROTOCOL_ID] = SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_UNKNOWN;

#define CHECK_NAME                                                                                                               \
    auto name = in_values[SSE_JSON_BODY][SSE_JSON_BODY_REINFORCEMENT_NAME].asString();                                           \
    auto reinforcement = NetworkReinforcementManager::get_instance()->get_reinforcement(name);                                   \
    if (!reinforcement)                                                                                                          \
    {                                                                                                                            \
        KLOG_DEBUG("The reinforcement %s isn't found.", name.c_str());                                                           \
        out_values[SSE_JSON_HEAD][SSE_JSON_HEAD_ERROR_CODE] = int32_t(SSEErrorCode::ERROR_PLUGIN_CONFIG_REINFORCEMENT_NOTFOUND); \
        break;                                                                                                                   \
    }

        switch (request_id)
        {
        case SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_GET_REQ:
        {
            CHECK_NAME;
            out_values[SSE_JSON_HEAD][SSE_JSON_HEAD_PROTOCOL_ID] = int32_t(SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_GET_RSP);
            std::string system_args;
            if (!reinforcement->get(system_args, error_code))
            {
                out_values[SSE_JSON_HEAD][SSE_JSON_HEAD_ERROR_CODE] = error_code;
                break;
            }
            out_values[SSE_JSON_BODY][SSE_JSON_BODY_REINFORCEMENT_SYSTEM_ARGS] = system_args;
            break;
        }
        case SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_SET_REQ:
        {
            CHECK_NAME;
            out_values[SSE_JSON_HEAD][SSE_JSON_HEAD_PROTOCOL_ID] = int32_t(SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_SET_RSP);
            auto args = in_values[SSE_JSON_BODY][SSE_JSON_BODY_REINFORCEMENT_ARGS];
            SSEErrorCode error_code = SSEErrorCode::SUCCESS;
            out_values[SSE_JSON_BODY]["result"] = reinforcement->set(StrUtils::json2str(args), error_code);
            out_values[SSE_JSON_HEAD][SSE_JSON_HEAD_ERROR_CODE] = int32_t(error_code);
            break;
        }
        default:
            out_values[SSE_JSON_HEAD][SSE_JSON_HEAD_ERROR_CODE] = int32_t(SSEErrorCode::ERROR_PLUGIN_CONFIG_UNSUPPORTED_REQ);
            break;
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        out_values[SSE_JSON_HEAD][SSE_JSON_HEAD_ERROR_CODE] = int32_t(SSEErrorCode::ERROR_PLUGIN_CONFIG_JSON_EXCEPTION);
    }

#undef CHECK_NAME

    return StrUtils::json2str(out_values);
}

}  // namespace Kiran
