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
 * Author:     chendingjian <chendingjian@kylinos.com.cn>
 */
#include "lib/base/error.h"
#include <QObject>

// 据说引用此头文件后，QString 的拼接操作会延后进行，达到 StringBuilder 的效果。
#include <QStringBuilder>

namespace KS
{
Error::Error()
{
}

QString Error::getErrorDesc(SSRErrorCode errorCode)
{
    QString errorDesc;
    switch (errorCode)
    {
    case SSRErrorCode::SUCCESS:
        errorDesc = QObject::tr("Success.");
        break;
    case SSRErrorCode::ERROR_COMMON_INVALID_ARGS:
        errorDesc = QObject::tr("Invalid args.");
        break;
    case SSRErrorCode::ERROR_TP_ADD_INVALID_FILE:
        errorDesc = QObject::tr("Added file types are not supported.");
        break;
    case SSRErrorCode::ERROR_TP_ADD_RECUR_FILE:
        errorDesc = QObject::tr("The file is already in the list, and there is no need to add it repeatedly.");
        break;
    case SSRErrorCode::ERROR_CHANGE_STORAGE_MODE_FAILED:
        errorDesc = QObject::tr("There is no trusted card or the trusted card is not supported.");
        break;
    case SSRErrorCode::ERROR_USER_PIN_ERROR:
        errorDesc = QObject::tr("The pin code is wrong!");
        break;
    case SSRErrorCode::ERROR_BM_DELETE_FAILED:
        errorDesc = QObject::tr("Failed to delete box.");
        break;
    case SSRErrorCode::ERROR_BM_MOUDLE_UNLOAD:
        errorDesc = QObject::tr("Failed to create box.");
        break;
    case SSRErrorCode::ERROR_BM_MKDIR_DATA_DIR_FAILED:
        errorDesc = QObject::tr("Insufficient free space or unknown error, box creation failed.");
        break;
    case SSRErrorCode::ERROR_BM_NOT_FOUND:
        errorDesc = QObject::tr("PrivateBox not found!");
        break;
    case SSRErrorCode::ERROR_BM_REPEATED_NAME:
        errorDesc = QObject::tr("The box is exist!");
        break;
    case SSRErrorCode::ERROR_BM_SETTINGS_SAME_PASSWORD:
        errorDesc = QObject::tr("The password set to the same as the current password is not supported.");
        break;
    case SSRErrorCode::ERROR_BM_UMOUNT_FAIL:
        errorDesc = QObject::tr("Busy resources!");
        break;
    case SSRErrorCode::ERROR_BM_MODIFY_PASSWORD_FAILED:
        errorDesc = QObject::tr("Failed to change the password, please check whether the password is correct.");
        break;
    case SSRErrorCode::ERROR_BM_INPUT_PASSWORD_ERROR:
        errorDesc = QObject::tr("Password error!");
        break;
    case SSRErrorCode::ERROR_BM_INPUT_PASSPHRASE_ERROR:
        errorDesc = QObject::tr("Passphrase error!");
        break;
    case SSRErrorCode::ERROR_BM_INTERNAL_ERRORS:
        errorDesc = QObject::tr("Internal error!");
        break;
    case SSRErrorCode::ERROR_DEVICE_INVALID_ID:
        errorDesc = QObject::tr("Invalid device.");
        break;
    case SSRErrorCode::ERROR_DEVICE_INVALID_PERM:
        errorDesc = QObject::tr("Invalid device permissions.");
        break;
    case SSRErrorCode::ERROR_DEVICE_INVALID_IFC_TYPE:
        errorDesc = QObject::tr("Invalid device interface type.");
        break;
    case SSRErrorCode::ERROR_DEVICE_DISABLE_HDMI:
        errorDesc = QObject::tr("The graphics card does not support HDMI interface shutdown.");
        break;
    case SSRErrorCode::ERROR_ACCOUNT_PASSWORD_ERROR:
        errorDesc = QObject::tr("Password error.");
        break;
    case SSRErrorCode::ERROR_ACCOUNT_BE_FREEZE:
        errorDesc = QObject::tr("This account has been freeze.");
        break;
    case SSRErrorCode::ERROR_ACCOUNT_BE_DIFF_NEW_PASSWORD:
        errorDesc = QObject::tr("New password must be different from old password.");
        break;
    case SSRErrorCode::ERROR_ACCOUNT_PERMISSION_DENIED:
        errorDesc = QObject::tr("Permission denied.");
        break;
    case SSRErrorCode::ERROR_ACCOUNT_UNKNOWN_ACCOUNT:
        errorDesc = QObject::tr("Unknown account.");
        break;
    case SSRErrorCode::ERROR_ACCOUNT_FAILED_SET_MULTI_FACTOR_AUTH_STATE:
        errorDesc = QObject::tr("Failed to Change Multi-Factor authentication state.");
        break;
    case SSRErrorCode::ERROR_LOG_GET_LOG_PAGE_ERROR:
        errorDesc = QObject::tr("per page limit must less than 100 and page index must greater than 0.");
        break;
    case SSRErrorCode::ERROR_TOOL_BOX_FAILED_SET_ACCESS_CONTROL:
        errorDesc = QObject::tr("Failed to set selinux status.");
        break;
    case SSRErrorCode::ERROR_TOOL_BOX_FAILED_SET_SECURITY_CONTEXT:
        errorDesc = QObject::tr("Failed to set Security Context.");
        break;
    case SSRErrorCode::ERROR_TOOL_BOX_FAILED_GET_SECURITY_CONTEXT:
        errorDesc = QObject::tr("Failed to get Security Context.");
        break;

    default:
        errorDesc = QObject::tr("Unknown error.");
        break;
    }

    errorDesc += QString(QObject::tr(" (error code: 0x%1)")).arg(QString::number((int)errorCode, 16));
    return errorDesc;
}

// 由于传递给DBus::Error的参数必须是不能被立即销毁的，因此将数据放入一个全局变量中，当然也可以放到成员变量中维护
std::string dbus_error_message;

BRError::BRError()
{
}

QString BRError::getErrorDesc(BRErrorCode errorCode)
{
    QString errorDesc;
    switch (errorCode)
    {
    case BRErrorCode::ERROR_DAEMON_STANDARD_TYPE_INVALID:
        errorDesc = QObject::tr("The standard type is invalid.");
        break;
    case BRErrorCode::ERROR_DAEMON_STRATEGY_TYPE_INVALID:
        errorDesc = QObject::tr("The strategy type is invalid.");
        break;
    case BRErrorCode::ERROR_DAEMON_NOTIFICATION_STATUS_INVALID:
        errorDesc = QObject::tr("The notification status is invalid.");
        break;
    case BRErrorCode::ERROR_DAEMON_RESOURCE_MONITOR_INVALID:
        errorDesc = QObject::tr("The resource monitor is invalid.");
        break;
    case BRErrorCode::ERROR_CUSTOM_RS_DECRYPT_FAILED:
    case BRErrorCode::ERROR_DAEMON_JSON2RS_FAILED:
    case BRErrorCode::ERROR_DAEMON_RS_CONTENT_INVALID:
        errorDesc = QObject::tr("Error format for reinforcement standard.");
        break;
    case BRErrorCode::ERROR_DAEMON_REINFORCEMENT_NOTFOUND:
        errorDesc = QObject::tr("Reinforcement item '{0}' is not found.");
        break;
    case BRErrorCode::ERROR_DAEMON_SCAN_IS_RUNNING:
    case BRErrorCode::ERROR_DAEMON_REINFORCE_IS_RUNNING:
        errorDesc = QObject::tr("The job is running, please don't repeat the operation.");
        break;
    case BRErrorCode::ERROR_DAEMON_FALLBACK_CANNOT_RUNNING:
        errorDesc = QObject::tr("The fallback is can't running, please wait for the reinforcement to be completed.");
        break;
    case BRErrorCode::ERROR_DAEMON_GET_RS_FAILED:
        errorDesc = QObject::tr("The standard reinforcement configuration is not found.");
        break;
    case BRErrorCode::ERROR_DAEMON_MACHINE_CODE_TRANS_FAILED:
        errorDesc = QObject::tr("Machine code error.");
        break;
    case BRErrorCode::ERROR_DAEMON_ACTIVATION_CODE_INVALID:
        errorDesc = QObject::tr("Activation code error.");
        break;
    case BRErrorCode::ERROR_DAEMON_SET_FALLBACK_RH_EMPTY:
        errorDesc = QObject::tr("There is no historical state, please reinforce it and operation.");
        break;
    case BRErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_1:
        errorDesc = QObject::tr("The subsequest configuration item scan progress has been cancelled.");
        break;
    case BRErrorCode::ERROR_DAEMON_CANCEL_NOTFOUND_JOB:
    case BRErrorCode::ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_2:
        errorDesc = QObject::tr("The subsequest configuration item reinforcement progress has been cancelled.");
        break;
    case BRErrorCode::ERROR_DAEMON_CONVERT_CATEGORIES2JSON_FAILED:
    case BRErrorCode::ERROR_DAEMON_CONVERT_PLUGINS2JSON_FAILED:
    case BRErrorCode::ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND:
    case BRErrorCode::ERROR_DAEMON_PLUGIN_INTERFACE_NOT_FOUND:
    case BRErrorCode::ERROR_DAEMON_SCAN_ALL_JOB_FAILED:
    case BRErrorCode::ERROR_CORE_REINFORCE_JOB_FAILED:
    case BRErrorCode::ERROR_DAEMON_SET_STANDARD_TYPE_FAILED:
    case BRErrorCode::ERROR_DAEMON_SET_STRATEGY_TYPE_FAILED:
    case BRErrorCode::ERROR_DAEMON_SET_TIME_SCAN_FAILED:
    case BRErrorCode::ERROR_DAEMON_SET_NOTIFICATION_STATUS_FAILED:
    case BRErrorCode::ERROR_PLUGIN_CONFIG_JSON_EXCEPTION:
    case BRErrorCode::ERROR_DAEMON_SCAN_RANGE_INVALID:
    case BRErrorCode::ERROR_PLUGIN_CONFIG_REINFORCEMENT_NOTFOUND:
    case BRErrorCode::ERROR_DAEMON_REINFORCE_RANGE_INVALID:
    case BRErrorCode::ERROR_DAEMON_SET_REINFORCEMENT_FAILED:
    case BRErrorCode::ERROR_PLUGIN_AUDIT_GET_JSON_ERROR:
    case BRErrorCode::ERROR_PLUGIN_AUDIT_SET_JSON_ERROR:
    case BRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENT_FAILED:
    case BRErrorCode::ERROR_DAEMON_GEN_REINFORCEMENTS_FAILED:
    case BRErrorCode::ERROR_DAEMON_PLUGIN_CALL_PYTHON_FUNC_FAILED:
        errorDesc = QObject::tr("Internel error.");
        break;
    case BRErrorCode::ERROR_DAEMON_SOFTWARE_UNACTIVATED:
        errorDesc = QObject::tr("The software is not activated.");
        break;
    default:
        errorDesc = QObject::tr("Unknown error.");
        break;
    }

    errorDesc += QString(QObject::tr(" (error code: 0x%1)")).arg(QString::number((int)errorCode, 16));
    return errorDesc;
}

}  // namespace KS
