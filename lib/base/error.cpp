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
#include "error.h"
#include <QObject>

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
        errorDesc = QObject::tr("Box not found!");
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

    default:
        errorDesc = QObject::tr("Unknown error.");
        break;
    }

    return errorDesc;
}

}  // namespace KS
