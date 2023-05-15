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

// KSS相关定义, 包括TP FP的接口
#define KSC_KSS_INIT_DBUS_OBJECT_PATH "/com/kylinsec/SC/KSS"
#define KSC_KSS_INIT_DBUS_INTERFACE_NAME "com.kylinsec.SC.KSS"

// kss命令 key相关定义 JK: JSON_KEY
#define KSC_KSS_JK_RES "res"
#define KSC_KSS_JK_DATA "data"
#define KSC_KSS_JK_COUNT "count"

#define KSC_KSS_JK_DATA_FILE_NAME "name"
#define KSC_KSS_JK_DATA_PATH "path"
#define KSC_KSS_JK_DATA_TYPE "ftype"
#define KSC_KSS_JK_DATA_STATUS "status"
#define KSC_KSS_JK_DATA_HASH "hash"
#define KSC_KSS_JK_DATA_ADD_TIME "time"
#define KSC_KSS_JK_DATA_GUARD "guard"

// 外设管理相关定义
#define KSC_DEVICE_MANAGER_DBUS_OBJECT_PATH "/com/kylinsec/SC/DeviceManager"

#define KSC_DEVICE_KEY_ID "id"
#define KSC_DEVICE_KEY_NAME "name"
#define KSC_DEVICE_KEY_TYPE "type"
#define KSC_DEVICE_KEY_INTERFACE_TYPE "interface_type"
#define KSC_DEVICE_KEY_READ "read"
#define KSC_DEVICE_KEY_WRITE "write"
#define KSC_DEVICE_KEY_EXECUTE "execute"
#define KSC_DEVICE_KEY_STATE "state"
#define KSC_DEVICE_KEY_ENABLE "enable"

    enum DeviceType
    {
        DEVICE_TYPE_DISK = 0,
        DEVICE_TYPE_CD,
        DEVICE_TYPE_MOUSE,
        DEVICE_TYPE_KEYBOARD,
        DEVICE_TYPE_NET_CARD,
        DEVICE_TYPE_WIRELESS_NET_CARD,
        DEVICE_TYPE_VIDEO,
        DEVICE_TYPE_AUDIO,
        DEVICE_TYPE_PRINTER,
        DEVICE_TYPE_HUB,
        DEVICE_TYPE_COMMUNICATIONS,
        DEVICE_TYPE_UNKNOWN
    };

    enum InterfaceType
    {
        INTERFACE_TYPE_USB = 0,
        INTERFACE_TYPE_BLUETOOTH,
        INTERFACE_TYPE_NET,
        INTERFACE_TYPE_HDMI,
        INTERFACE_TYPE_UNKNOWN
    };

    enum DeviceState
    {
        DEVICE_STATE_ENABLE = 0,
        DEVICE_STATE_DISABLE,
        DEVICE_STATE_UNAUTHORIED
    };

    enum DeviceAction
    {
        DEVICE_ACTION_ADD = 0,
        DEVICE_ACTION_REMOVE,
        DEVICE_ACTION_CHANGE
    };

#ifdef __cplusplus
}
#endif
