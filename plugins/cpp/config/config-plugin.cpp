/**
 * @file          /kiran-sse-manager/plugins/cpp/config/config-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/cpp/config/config-plugin.h"
#include <json/json.h>
#include "lib/base/base.h"
#include "plugins/cpp/config/cr-manager.h"

namespace Kiran
{
PLUGIN_EXPORT_FUNC_DEF(SSEPluginConfig);

void SSEPluginConfig::activate()
{
    CRManager::global_init();
}

void SSEPluginConfig::deactivate()
{
    CRManager::global_deinit();
}

std::string SSEPluginConfig::execute(const std::string& in_json)
{
    auto in_values = StrUtils::str2json(in_json);
    Json::Value out_values;

    try
    {
        auto request_id = in_values["head"]["id"].asInt();

        KLOG_DEBUG("request id: %d.", request_id);

        out_values["head"]["id"] = SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_UNKNOWN;

#define CHECK_NAME                                                                                            \
    auto name = in_values["body"]["name"].asString();                                                         \
    auto reinforcement = CRManager::get_instance()->get_reinforcement(name);                                  \
    if (!reinforcement)                                                                                       \
    {                                                                                                         \
        KLOG_DEBUG("The reinforcement %s isn't found.", name.c_str());                                        \
        out_values["head"]["error_code"] = int32_t(SSEErrorCode::ERROR_PLUGIN_CONFIG_REINFORCEMENT_NOTFOUND); \
        break;                                                                                                \
    }

        switch (request_id)
        {
        case SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_RA_MATCH_RS_REQ:
        {
            CHECK_NAME;
            out_values["head"]["id"] = int32_t(SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_RA_MATCH_RS_RSP);
            auto rules = in_values["body"]["rules"];
            auto args = in_values["body"]["args"];
            out_values["body"]["match"] = reinforcement->RAMatchRS(StrUtils::json2str(rules), StrUtils::json2str(args));
            break;
        }
        case SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_GET_REQ:
        {
            CHECK_NAME;
            out_values["head"]["id"] = int32_t(SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_GET_RSP);
            auto rules = in_values["head"]["body"]["rules"];
            out_values["body"]["match"] = reinforcement->SCMatchRS(StrUtils::json2str(rules));
            break;
        }
        case SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_SET_REQ:
        {
            CHECK_NAME;
            out_values["head"]["id"] = int32_t(SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_SET_RSP);
            auto args = in_values["head"]["body"]["args"];
            SSEErrorCode error_code = SSEErrorCode::SUCCESS;
            out_values["body"]["result"] = reinforcement->Reinforce(StrUtils::json2str(args), error_code);
            out_values["head"]["error_code"] = int32_t(error_code);
            break;
        }
        default:
            out_values["head"]["error_code"] = int32_t(SSEErrorCode::ERROR_PLUGIN_CONFIG_UNSUPPORTED_REQ);
            break;
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        out_values["head"]["error_code"] = int32_t(SSEErrorCode::ERROR_PLUGIN_CONFIG_JSON_EXCEPTION);
    }

#undef CHECK_NAME

    return StrUtils::json2str(out_values);
}

}  // namespace Kiran
