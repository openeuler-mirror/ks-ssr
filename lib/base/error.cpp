/**
 * @file          /ks-ssr-manager/lib/base/error.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/base/error.h"

#include <fmt/format.h>
#include <glib/gi18n.h>

namespace KS
{
SSRError::SSRError()
{
}

std::string SSRError::get_error_desc(SSRErrorCode error_code)
{
    std::string error_desc;
    switch (error_code)
    {
    case SSRErrorCode::ERROR_DAEMON_STANDARD_TYPE_INVALID:
        error_desc = _("The standard type is invalid.");
        break;
    // case SSRErrorCode::ERROR_CORE_RS_NOT_FOUND:
    //     error_desc = _("The reinforcement standard file is not found.");
    //     break;
    case SSRErrorCode::ERROR_CUSTOM_RS_DECRYPT_FAILED:
    case SSRErrorCode::ERROR_DAEMON_JSON2RS_FAILED:
    case SSRErrorCode::ERROR_DAEMON_RS_CONTENT_INVALID:
        error_desc = _("Error format for reinforcement standard.");
        break;
    case SSRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND:
        error_desc = _("Reinforcement item '{0}' is not found.");
        break;
    case SSRErrorCode::ERROR_DAEMON_SCAN_IS_RUNNING:
    case SSRErrorCode::ERROR_DAEMON_REINFORCE_IS_RUNNING:
        error_desc = _("The job is running, please don't repeat the operation.");
        break;
    case SSRErrorCode::ERROR_DAEMON_GET_RS_FAILED:
        error_desc = _("The standard reinforcement configuration is not found.");
        break;
    case SSRErrorCode::ERROR_DAEMON_MACHINE_CODE_TRANS_FAILED:
        error_desc = _("Machine code error.");
        break;
    case SSRErrorCode::ERROR_DAEMON_ACTIVATION_CODE_INVALID:
        error_desc = _("Activation code error.");
        break;
    case SSRErrorCode::ERROR_DAEMON_CONVERT_CATEGORIES2JSON_FAILED:
    case SSRErrorCode::ERROR_DAEMON_CONVERT_PLUGINS2JSON_FAILED:
    case SSRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND:
    case SSRErrorCode::ERROR_DAEMON_PLUGIN_INTERFACE_NOT_FOUND:
    case SSRErrorCode::ERROR_DAEMON_SCAN_ALL_JOB_FAILED:
    case SSRErrorCode::ERROR_CORE_REINFORCE_JOB_FAILED:
    case SSRErrorCode::ERROR_DAEMON_SET_STANDARD_TYPE_FAILED:
    case SSRErrorCode::ERROR_PLUGIN_CONFIG_JSON_EXCEPTION:
    case SSRErrorCode::ERROR_DAEMON_SCAN_RANGE_INVALID:
    case SSRErrorCode::ERROR_PLUGIN_CONFIG_REINFORCEMENT_NOTFOUND:
    case SSRErrorCode::ERROR_DAEMON_REINFORCE_RANGE_INVALID:
    case SSRErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_1:
    case SSRErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_2:
    case SSRErrorCode::ERROR_DAEMON_CANCEL_NOTFOUND_JOB:
    case SSRErrorCode::ERROR_DAEMON_SET_REINFORCEMENT_FAILED:
    case SSRErrorCode::ERROR_PLUGIN_AUDIT_GET_JSON_ERROR:
    case SSRErrorCode::ERROR_PLUGIN_AUDIT_SET_JSON_ERROR:
    case SSRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENT_FAILED:
    case SSRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENTS_FAILED:
    case SSRErrorCode::ERROR_DAEMON_PLUGIN_CALL_PYTHON_FUNC_FAILED:
        error_desc = _("Internel error.");
        break;
    case SSRErrorCode::ERROR_DAEMON_SOFTWARE_UNACTIVATED:
        error_desc = _("The software is not activated.");
        break;
    default:
        error_desc = _("Unknown error.");
        break;
    }

    error_desc += fmt::format(_(" (error code: 0x{:x})"), int32_t(error_code));
    return error_desc;
}
}  // namespace KS
