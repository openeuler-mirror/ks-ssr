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

// 授权相关定义
#define KSC_LICENSE_DBUS_OBJECT_PATH "/com/kylinsec/SC/License"
#define KSC_LICENSE_DBUS_INTERFACE_NAME "com.kylinsec.SC.License"

// 保密箱相关定义
#define KSC_BOX_MANAGER_DBUS_OBJECT_PATH "/com/kylinsec/SC/BoxManager"
#define KSC_BOX_MANAGER_DBUS_INTERFACE_NAME "com.kylinsec.SC.BoxManager"

// BM: Box Manager
#define KSC_BM_JK_BOX_UID "uid"
#define KSC_BM_JK_BOX_NAME "name"
#define KSC_BM_JK_BOX_MOUNTED "mounted"

// 保险箱名称长度以及保险箱密码长度
#define KSC_BOX_NAME_MAX_LENGTH 16
#define KSC_BOX_PASSWORD_MAX_LENGTH 16

// KSS相关定义, 包括TP FP的接口
#define KSC_KSS_INIT_DBUS_OBJECT_PATH "/com/kylinsec/SC/KSS"
#define KSC_KSS_INIT_DBUS_INTERFACE_NAME "com.kylinsec.SC.KSS"

// kss命令 key相关定义 JK: JSON_JK
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

    enum KSCKSSTrustedFileType
    {
        KSC_KSS_TRUSTED_FILE_TYPE_EXECUTE = 0,
        KSC_KSS_TRUSTED_FILE_TYPE_KERNEL,
        KSC_KSS_TRUSTED_FILE_TYPE_NONE
    };

    enum KSCKSSTrustedStorageType
    {
        KSC_KSS_TRUSTED_STORAGE_TYPE_SOFT = 0,
        KSC_KSS_TRUSTED_STORAGE_TYPE_HARD,
        KSC_KSS_TRUSTED_STORAGE_TYPE_NONE
    };

// 外设管理相关定义
#define KSC_DEVICE_MANAGER_DBUS_OBJECT_PATH "/com/kylinsec/SC/DeviceManager"
#define KSC_DEVICE_MANAGER_DBUS_INTERFACE_NAME "com.kylinsec.SC.DeviceManager"

#define KSC_DEVICE_JK_ID "id"
#define KSC_DEVICE_JK_NAME "name"
#define KSC_DEVICE_JK_TYPE "type"
#define KSC_DEVICE_JK_INTERFACE_TYPE "interface_type"
#define KSC_DEVICE_JK_READ "read"
#define KSC_DEVICE_JK_WRITE "write"
#define KSC_DEVICE_JK_EXECUTE "execute"
#define KSC_DEVICE_JK_STATE "state"
#define KSC_DEVICE_JK_ENABLE "enable"

#define KSC_DCR_JK_NAME "name"
#define KSC_DCR_JK_TYPE "type"
#define KSC_DCR_JK_TIME "time"
#define KSC_DCR_JK_STATE "state"

#define KSC_DI_JK_TYPE "type"
#define KSC_DI_JK_ENABLE "enable"

#define KSC_PERMISSION_AUTHENTICATION "com.kylinsec.SC.PermissionAuthentication"

    enum DeviceType
    {
        DEVICE_TYPE_UNKNOWN = 0,
        DEVICE_TYPE_STORAGE,
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
        DEVICE_TYPE_LAST,
    };

    enum InterfaceType
    {
        INTERFACE_TYPE_UNKNOWN = 0,
        INTERFACE_TYPE_USB,
        INTERFACE_TYPE_BLUETOOTH,
        INTERFACE_TYPE_NET,
        INTERFACE_TYPE_HDMI,
        INTERFACE_TYPE_USB_KBD,
        INTERFACE_TYPE_USB_MOUSE,
        INTERFACE_TYPE_LAST,
    };

    enum DeviceState
    {
        DEVICE_STATE_UNAUTHORIED = 0,
        DEVICE_STATE_ENABLE,
        DEVICE_STATE_DISABLE,
    };

    enum DeviceAction
    {
        DEVICE_ACTION_ADD = 0,
        DEVICE_ACTION_REMOVE,
        DEVICE_ACTION_CHANGE
    };

    enum DeviceConnectState
    {
        DEVICE_CONNECT_SUCCESSED = 0,
        DEVICE_CONNECT_FAILED
    };

    /**
     * @brief 设备权限
     */
    enum PermissionType
    {
        PERMISSION_TYPE_READ = (1 << 0),
        PERMISSION_TYPE_WRITE = (1 << 1),
        PERMISSION_TYPE_EXEC = (1 << 2)
    };
#ifdef __cplusplus
}
#endif
