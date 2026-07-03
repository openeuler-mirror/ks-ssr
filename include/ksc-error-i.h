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
        ERROR_CHANGE_STORAGE_MODE_FAILED,

        // BM Box manager
        ERROR_BM_DELETE_FAILED,
        ERROR_BM_MOUDLE_UNLOAD,
        ERROR_BM_NOT_FOUND,
        ERROR_BM_MODIFY_PASSWORD_FAILED,
        ERROR_BM_INPUT_PASSWORD_ERROR,
        ERROR_BM_INPUT_PASSPHRASE_ERROR
    };

#ifdef __cplusplus
}
#endif
