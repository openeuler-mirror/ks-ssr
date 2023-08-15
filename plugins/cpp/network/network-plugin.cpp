/**
 * @file          /kiran-sse-manager/plugins/cpp/network/network-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/cpp/config/network-plugin.h"
#include <json/json.h>
#include "lib/base/base.h"

namespace Kiran
{
PLUGIN_EXPORT_FUNC_DEF(SSEPluginConfig);

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

    try
    {
        switch (in_values["head"]["id"].asInt())
        {
        case SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_RA_MATCH_RS_REQ:
            break;
        default:
            break;
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARNING("%s.", e.what());
    }

    out_values["head"]["id"] = SSEPluginProtocol::SSE_PLUGIN_PROTOCOL_UNKNOWN;
    return StrUtils::json2str(out_values);
}

}  // namespace Kiran
