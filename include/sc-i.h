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

#define SC_DBUS_NAME "com.kylinsec.SC"
#define SC_DBUS_OBJECT_PATH "/com/kylinsec/SC"
#define SC_DBUS_INTERFACE_NAME "com.kylinsec.SC"

// 保密箱相关定义
#define SC_BOX_MANAGER_DBUS_OBJECT_PATH "/com/kylinsec/SC/BoxManager"
#define SC_BOX_MANAGER_DBUS_INTERFACE_NAME "com.kylinsec.SC.BoxManager"

// BM: Box Manager
#define SCBM_JK_BOX_UID "uid"
#define SCBM_JK_BOX_NAME "name"
#define SCBM_JK_BOX_MOUNTED "isMount"

#ifdef __cplusplus
}
#endif
