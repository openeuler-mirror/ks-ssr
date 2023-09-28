/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
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

    enum KSCErrorCode
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

        // BM Box manager
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

        //Device manager
        ERROR_DEVICE_INVALID_ID,
        ERROR_DEVICE_INVALID_PERM,
        ERROR_DEVICE_INVALID_IFC_TYPE,
        ERROR_DEVICE_DISABLE_HDMI,

    };

#ifdef __cplusplus
}
#endif
