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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define KSC_DBUS_NAME "com.kylinsec.SC"
#define KSC_DBUS_OBJECT_PATH "/com/kylinsec/SC"
#define KSC_DBUS_INTERFACE_NAME "com.kylinsec.SC"

// 保密箱相关定义
#define KSC_BOX_MANAGER_DBUS_OBJECT_PATH "/com/kylinsec/SC/BoxManager"
#define KSC_BOX_MANAGER_DBUS_INTERFACE_NAME "com.kylinsec.SC.BoxManager"

// BM: Box Manager
#define KSC_BM_JK_BOX_UID "uid"
#define KSC_BM_JK_BOX_NAME "name"
#define KSC_BM_JK_BOX_MOUNTED "mounted"

// 可信保护相关定义
#define KSC_TRUSTED_PROTECTED_DBUS_OBJECT_PATH "/com/kylinsec/SC/Trusted"
#define KSC_TRUSTED_PROTECTED_DBUS_INTERFACE_NAME "com.kylinsec.SC.Trusted"

// 文件保护相关定义
#define KSC_FILE_PROTECTED_DBUS_OBJECT_PATH "/com/kylinsec/SC/FileProtected"
#define KSC_FILE_PROTECTED_DBUS_INTERFACE_NAME "com.kylinsec.SC.FileProtected"

// kss命令 key相关定义 JK: JSON_KEY
#define KSC_JK_RES "res"
#define KSC_JK_DATA "data"
#define KSC_JK_COUNT "count"

#define KSC_JK_DATA_FILE_NAME "name"
#define KSC_JK_DATA_PATH "path"
#define KSC_JK_DATA_TYPE "ftype"
#define KSC_JK_DATA_STATUS "status"
#define KSC_JK_DATA_HASH "hash"
#define KSC_JK_DATA_ADD_TIME "time"

#ifdef __cplusplus
}
#endif
