/**
 * @file          /kiran-ssr-manager/plugins/cpp/audit/audit-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/cpp/audit/audit-plugin.h"
#include <json/json.h>
#include "lib/base/base.h"
#include "plugins/cpp/audit/reinforcement-manager.h"

PLUGIN_EXPORT_FUNC_DEF(Kiran::Audit::SSRPluginAudit);

namespace Kiran
{
namespace Audit
{
void SSRPluginAudit::activate()
{
}

void SSRPluginAudit::deactivate()
{
}

std::string SSRPluginAudit::execute(const std::string& in_json)
{
    auto in_values = StrUtils::str2json(in_json);
    Json::Value out_values;
    SSRErrorCode error_code = SSRErrorCode::SUCCESS;

    try
    {
        auto request_id = in_values[SSR_JSON_HEAD][SSR_JSON_HEAD_PROTOCOL_ID].asInt();

        KLOG_DEBUG("request id: %d.", request_id);

        out_values[SSR_JSON_HEAD][SSR_JSON_HEAD_PROTOCOL_ID] = SSRPluginProtocol::SSR_PLUGIN_PROTOCOL_UNKNOWN;

#define CHECK_NAME                                                                                                               \
    auto name = in_values[SSR_JSON_BODY][SSR_JSON_BODY_REINFORCEMENT_NAME].asString();                                           \
    auto reinforcement = ReinforcementManager::get_instance()->get_reinforcement(name);                                          \
    if (!reinforcement)                                                                                                          \
    {                                                                                                                            \
        KLOG_DEBUG("The reinforcement %s isn't found.", name.c_str());                                                           \
        out_values[SSR_JSON_HEAD][SSR_JSON_HEAD_ERROR_CODE] = int32_t(SSRErrorCode::ERROR_PLUGIN_CONFIG_REINFORCEMENT_NOTFOUND); \
        break;                                                                                                                   \
    }

        switch (request_id)
        {
        case SSRPluginProtocol::SSR_PLUGIN_PROTOCOL_GET_REQ:
        {
            CHECK_NAME;
            out_values[SSR_JSON_HEAD][SSR_JSON_HEAD_PROTOCOL_ID] = int32_t(SSRPluginProtocol::SSR_PLUGIN_PROTOCOL_GET_RSP);
            std::string system_args;
            if (!reinforcement->get(system_args, error_code))
            {
                out_values[SSR_JSON_HEAD][SSR_JSON_HEAD_ERROR_CODE] = error_code;
                break;
            }
            out_values[SSR_JSON_BODY][SSR_JSON_BODY_REINFORCEMENT_SYSTEM_ARGS] = system_args;
            break;
        }
        case SSRPluginProtocol::SSR_PLUGIN_PROTOCOL_SET_REQ:
        {
            CHECK_NAME;
            out_values[SSR_JSON_HEAD][SSR_JSON_HEAD_PROTOCOL_ID] = int32_t(SSRPluginProtocol::SSR_PLUGIN_PROTOCOL_SET_RSP);
            auto args = in_values[SSR_JSON_BODY][SSR_JSON_BODY_REINFORCEMENT_ARGS];
            SSRErrorCode error_code = SSRErrorCode::SUCCESS;
            out_values[SSR_JSON_BODY]["result"] = reinforcement->set(StrUtils::json2str(args), error_code);
            out_values[SSR_JSON_HEAD][SSR_JSON_HEAD_ERROR_CODE] = int32_t(error_code);
            break;
        }
        default:
            out_values[SSR_JSON_HEAD][SSR_JSON_HEAD_ERROR_CODE] = int32_t(SSRErrorCode::ERROR_PLUGIN_CONFIG_UNSUPPORTED_REQ);
            break;
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        out_values[SSR_JSON_HEAD][SSR_JSON_HEAD_ERROR_CODE] = int32_t(SSRErrorCode::ERROR_PLUGIN_CONFIG_JSON_EXCEPTION);
    }

#undef CHECK_NAME

    return StrUtils::json2str(out_values);
}

}  // namespace Audit
}  // namespace Kiran
