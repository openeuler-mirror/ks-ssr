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

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    enum class SSRErrorCode
    {
        // Common
        SUCCESS,
        ERROR_FAILED,
        ERROR_COMMON_INVALID_ARGS,

        // KSS, FP and TP Trusted protect
        ERROR_TP_ADD_INVALID_FILE,
        ERROR_TP_ADD_RECUR_FILE,
        ERROR_CHANGE_STORAGE_MODE_FAILED,
        ERROR_USER_PIN_ERROR,

        // BM PrivateBox manager
        ERROR_BM_DELETE_FAILED,
        ERROR_BM_MOUDLE_UNLOAD,
        ERROR_BM_MKDIR_DATA_DIR_FAILED,
        ERROR_BM_NOT_FOUND,
        ERROR_BM_REPEATED_NAME,
        ERROR_BM_SETTINGS_SAME_PASSWORD,
        ERROR_BM_UMOUNT_FAIL,
        ERROR_BM_MODIFY_PASSWORD_FAILED,
        ERROR_BM_INPUT_PASSWORD_ERROR,
        ERROR_BM_INPUT_PASSPHRASE_ERROR,
        ERROR_BM_INTERNAL_ERRORS,

        // Device manager
        ERROR_DEVICE_INVALID_ID,
        ERROR_DEVICE_INVALID_PERM,
        ERROR_DEVICE_INVALID_IFC_TYPE,
        ERROR_DEVICE_DISABLE_HDMI,

    };

    enum class BRErrorCode
    {
        // Common
        SUCCESS,
        ERROR_FAILED,

        // Main
        ERROR_PLUGIN_NOT_EXIST_1 = 0x00100,
        // ERROR_SET_STANDARD_TYPE_FAILED,

        // core
        ERROR_CUSTOM_RS_DECRYPT_FAILED,
        ERROR_CORE_REINFORCE_JOB_FAILED,

        // deamon
        ERROR_DAEMON_STANDARD_TYPE_INVALID,
        ERROR_DAEMON_SET_STANDARD_TYPE_FAILED,
        ERROR_DAEMON_STRATEGY_TYPE_INVALID,
        ERROR_DAEMON_SET_STRATEGY_TYPE_FAILED,
        ERROR_DAEMON_SET_TIME_SCAN_FAILED,
        ERROR_DAEMON_NOTIFICATION_STATUS_INVALID,
        ERROR_DAEMON_SET_NOTIFICATION_STATUS_FAILED,
        ERROR_DAEMON_RESOURCE_MONITOR_INVALID,
        ERROR_DAEMON_SET_RESOURCE_MONITOR_FAILED,
        ERROR_DAEMON_FALLBACK_STATUS_INVALID,
        ERROR_DAEMON_SET_FALLBACK_STATUS_FAILED,
        ERROR_DAEMON_SET_FALLBACK_RH_EMPTY,
        ERROR_DAEMON_CONVERT_CATEGORIES2JSON_FAILED,
        ERROR_DAEMON_GET_RS_FAILED,
        ERROR_DAEMON_CONVERT_PLUGINS2JSON_FAILED,
        ERROR_DAEMON_JSON2RS_FAILED,
        ERROR_DAEMON_RS_CONTENT_INVALID,
        ERROR_DAEMON_SET_REINFORCEMENT_FAILED,
        ERROR_DAEMON_REINFORCEMENT_NOTFOUND,
        ERROR_DAEMON_GEN_REINFORCEMENT_FAILED,
        ERROR_DAEMON_GEN_REINFORCEMENTS_FAILED,
        ERROR_DAEMON_SCAN_ALL_JOB_FAILED,
        ERROR_DAEMON_SCAN_IS_RUNNING,
        ERROR_DAEMON_PLUGIN_OF_REINFORCEMENT_NOT_FOUND,
        ERROR_DAEMON_PLUGIN_INTERFACE_NOT_FOUND,
        ERROR_DAEMON_SCAN_RANGE_INVALID,
        ERROR_DAEMON_REINFORCE_IS_RUNNING,
        ERROR_DAEMON_REINFORCE_RANGE_INVALID,
        ERROR_DAEMON_FALLBACK_CANNOT_RUNNING,
        ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_1,
        ERROR_DAEMON_CANCEL_CANNOT_CANCELLED_2,
        ERROR_DAEMON_CANCEL_NOTFOUND_JOB,
        ERROR_DAEMON_PLUGIN_CALL_PYTHON_FUNC_FAILED,
        ERROR_DAEMON_MACHINE_CODE_TRANS_FAILED,
        ERROR_DAEMON_ACTIVATION_CODE_INVALID,
        ERROR_DAEMON_SOFTWARE_UNACTIVATED,

        // plugins

        // plugin audit
        ERROR_PLUGIN_AUDIT_GET_JSON_ERROR,
        ERROR_PLUGIN_AUDIT_SET_JSON_ERROR,

        // plugin config
        ERROR_PLUGIN_CONFIG_REINFORCEMENT_NOTFOUND,
        ERROR_PLUGIN_CONFIG_UNSUPPORTED_REQ,
        ERROR_PLUGIN_CONFIG_JSON_EXCEPTION,

    };
#ifdef __cplusplus
}
#endif
