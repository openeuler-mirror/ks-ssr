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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once
#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SSR_DBUS_NAME "com.kylinsec.SSR"
#define SSR_DBUS_OBJECT_PATH "/com/kylinsec/SSR"
#define SSR_DBUS_INTERFACE_NAME "com.kylinsec.SSR"

// 用户名定义
#define SSR_ACCOUNT_NAME_SYSADM "sysadm"
#define SSR_ACCOUNT_NAME_SECADM "secadm"
#define SSR_ACCOUNT_NAME_AUDADM "audadm"

#define SSR_ACCOUNT_NAME_COMADM "comadm"

// 用户名称长度以及保险箱密码长度
#define SSR_USER_NAME_MAX_LENGTH 16
#define SSR_PASSWORD_MAX_LENGTH 16

// 用户管理相关定义
#define SSR_ACCOUNT_DBUS_OBJECT_PATH "/com/kylinsec/SSR/Account"
#define SSR_ACCOUNT_BUS_INTERFACE_NAME "com.kylinsec.SSR.Account"

// 保密箱相关定义
#define SSR_BOX_MANAGER_DBUS_OBJECT_PATH "/com/kylinsec/SSR/PrivateBox"
#define SSR_BOX_MANAGER_DBUS_INTERFACE_NAME "com.kylinsec.SSR.PrivateBox"

// BM: PrivateBox Manager
#define SSR_BM_JK_BOX_UID "uid"
#define SSR_BM_JK_BOX_NAME "name"
#define SSR_BM_JK_BOX_MOUNTED "mounted"

// KSS相关定义, 包括TP FP的接口
#define SSR_KSS_INIT_DBUS_OBJECT_PATH "/com/kylinsec/SSR/KSS"
#define SSR_KSS_INIT_DBUS_INTERFACE_NAME "com.kylinsec.SSR.KSS"

// kss命令 key相关定义 JK: JSON_JK
#define SSR_KSS_JK_RES "res"
#define SSR_KSS_JK_DATA "data"
#define SSR_KSS_JK_COUNT "count"

#define SSR_KSS_JK_DATA_FILE_NAME "name"
#define SSR_KSS_JK_DATA_PATH "path"
#define SSR_KSS_JK_DATA_TYPE "ftype"
#define SSR_KSS_JK_DATA_STATUS "status"
#define SSR_KSS_JK_DATA_HASH "hash"
#define SSR_KSS_JK_DATA_ADD_TIME "time"
#define SSR_KSS_JK_DATA_GUARD "guard"

// BR 相关定义
// TODO : 改成SSR_BR
#define BR_DBUS_NAME "com.kylinsec.SSR.BR"
#define BR_DBUS_OBJECT_PATH "/com/kylinsec/SSR/BR"
#define BR_DBUS_INTERFACE_NAME "com.kylinsec.SSR.BR"

#define SSR_BR_CUSTOM_RA_STRATEGY_FILEPATH SSR_INSTALL_DATADIR "/br-custom-ra-strategy.xml"
#define SSR_BR_CUSTOM_RA_FILEPATH SSR_INSTALL_DATADIR "/br-custom-ra.xml"

#define SSR_TOOL_BOX_DBUS_NAME "com.kylinsec.SSR.ToolBox"
#define SSR_TOOL_BOX_DBUS_OBJECT_PATH "/com/kylinsec/SSR/ToolBox"

    enum SSRKSSTrustedFileType
    {
        SSR_KSS_TRUSTED_FILE_TYPE_EXECUTE = 0,
        SSR_KSS_TRUSTED_FILE_TYPE_KERNEL,
        SSR_KSS_TRUSTED_FILE_TYPE_NONE
    };

    enum SSRKSSTrustedStorageType
    {
        SSR_KSS_TRUSTED_STORAGE_TYPE_SOFT = 0,
        SSR_KSS_TRUSTED_STORAGE_TYPE_HARD,
        SSR_KSS_TRUSTED_STORAGE_TYPE_NONE
    };

// 外设管理相关定义
#define SSR_DEVICE_MANAGER_DBUS_OBJECT_PATH "/com/kylinsec/SSR/DeviceManager"
#define SSR_DEVICE_MANAGER_DBUS_INTERFACE_NAME "com.kylinsec.SSR.DeviceManager"

#define SSR_DEVICE_JK_ID "id"
#define SSR_DEVICE_JK_NAME "name"
#define SSR_DEVICE_JK_TYPE "type"
#define SSR_DEVICE_JK_INTERFACE_TYPE "interface_type"
#define SSR_DEVICE_JK_READ "read"
#define SSR_DEVICE_JK_WRITE "write"
#define SSR_DEVICE_JK_EXECUTE "execute"
#define SSR_DEVICE_JK_STATE "state"
#define SSR_DEVICE_JK_ENABLE "enable"

#define SSR_DCR_JK_NAME "name"
#define SSR_DCR_JK_TYPE "type"
#define SSR_DCR_JK_TIME "time"
#define SSR_DCR_JK_STATE "state"

#define SSR_DI_JK_TYPE "type"
#define SSR_DI_JK_ENABLE "enable"

#define SSR_PERMISSION_AUTHENTICATION "com.kylinsec.SSR.PermissionAuthentication"

    enum OsUserType
    {
        USER_TYPE_MANAGER = 0,
        USER_TYPE_NORMAL
    };

    enum LogResult
    {
        LOG_RESULT_FALSE = 0,
        LOG_RESULT_TRUE,
        LOG_RESULT_ALL,
    };

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
        DEVICE_TYPE_BLUETOOTH,
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

    // 加固标准类型 加固标准对应着各个加固项，有的加固项用户不需要，就可以自定义导入
    enum BRStandardType
    {
        // 系统默认标准 显示所有加固项，可对所有加固项进行操作
        BR_STANDARD_TYPE_SYSTEM = 0,
        // 用户导入的标准（自定义加固标准）显示部分加固项，可对自定义导入的加固项进行操作
        BR_STANDARD_TYPE_CUSTOM,
        BR_STANDARD_TYPE_LAST
    };

    // 加固策略类型 加固策略相当于一个动作类型，可以保留用户的操作（设置加固参数、勾选复选框）
    enum BRStrategyType
    {
        // 系统默认加固策略 所有加固项默认选中并使用默认加固参数
        BR_STRATEGY_TYPE_SYSTEM = 0,
        // 用户导入的策略（自定义加固策略）用户可自定义加固参数，勾选状态，可导入导出
        BR_STRATEGY_TYPE_CUSTOM,
        BR_STRATEGY_TYPE_LAST
    };

    // 前台通知提示 开启/关闭
    enum BRNotificationStatus
    {
        // 开启通知
        BR_NOTIFICATION_STATUS_OPEN = 0,
        // 关闭通知
        BR_NOTIFICATION_STATUS_CLOSE,
        BR_NOTIFICATION_STATUS_OTHER
    };

    // 加固项状态
    enum BRReinforcementState
    {
        // 未知
        BR_REINFORCEMENT_STATE_UNKNOWN = 0,
        // 符合标准
        BR_REINFORCEMENT_STATE_SAFE = (1 << 0),
        // 不符合标准
        BR_REINFORCEMENT_STATE_UNSAFE = (1 << 1),
        // 未扫描，加固项还未扫描任务就被取消则会变为该状态
        BR_REINFORCEMENT_STATE_UNSCAN = (1 << 2),
        // 扫描中
        BR_REINFORCEMENT_STATE_SCANNING = (1 << 3),
        // 扫描错误
        BR_REINFORCEMENT_STATE_SCAN_ERROR = (1 << 4),
        // 扫描完成
        BR_REINFORCEMENT_STATE_SCAN_DONE = (1 << 5),
        // 未加固，加固项还未加固任务就被取消则会变为该状态
        BR_REINFORCEMENT_STATE_UNREINFORCE = (1 << 6),
        // 加固错误
        BR_REINFORCEMENT_STATE_REINFORCE_ERROR = (1 << 7),
        // 加固中
        BR_REINFORCEMENT_STATE_REINFORCING = (1 << 8),
        // 加固完成
        BR_REINFORCEMENT_STATE_REINFORCE_DONE = (1 << 9)
    };

    // 加固类型
    enum BRReinforceType
    {
        // 	使用默认方式加固
        SEE_REINFORCE_TYPE_DEFAULT = 0,
        // 使用自定义参数加固
        SEE_REINFORCE_TYPE_CUSTOM
    };

    // 任务状态
    enum BRJobState
    {
        // 空闲
        BR_JOB_STATE_IDLE,
        // 执行中
        BR_JOB_STATE_RUNNING,
        // 执行完成
        BR_JOB_STATE_DONE,
        // 执行完成(中途被取消)
        BR_JOB_STATE_CANCEL_DONE
    };

    // 资源监控
    enum BRResourceMonitor
    {
        // 关闭
        BR_RESOURCE_MONITOR_CLOSE = 0,
        // 开启
        BR_RESOURCE_MONITOR_OPEN,
        // 其它
        BR_RESOURCE_MONITOR_OTHER
    };

    // 回退方式
    enum BRFallbackMethod
    {
        // 回到初始状态
        BR_FALLBACK_METHOD_INITIAL = 0,
        // 回到上一次加固
        BR_FALLBACK_METHOD_LAST,
        // 其它
        BR_FALLBACK_METHOD_OTHER
    };

    // 回退状态
    enum BRFallbackStatus
    {
        // 回退未开始
        BR_FALLBACK_STATUS_NOT_STARTED = 0,
        // 回退进行中
        BR_FALLBACK_STATUS_IN_PROGRESS,
        // 回退完成
        BR_FALLBACK_STATUS_IS_FINISHED
    };

    enum LOGAlertType
    {
        HAZARD_BEHAVIOR,
        ATTACK_DETECT
    };

#ifdef __cplusplus
}
#endif
