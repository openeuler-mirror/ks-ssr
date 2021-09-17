/**
 * @file          /kiran-ssr-manager/include/ssr-i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <error-i.h>
#include <plugin-i.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SSR_DBUS_NAME "com.kylinsec.Kiran.SSR"
#define SSR_DBUS_OBJECT_PATH "/com/kylinsec/Kiran/SSR"
#define SSR_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SSR"

    // 加固标准类型
    enum SSRStandardType
    {
        // 系统默认标准
        SSR_STANDARD_TYPE_SYSTEM = 0,
        // 用户导入的标准（自定义加固标准）
        SSR_STANDARD_TYPE_CUSTOM,
        SSR_STANDARD_TYPE_LAST,
    };

    // 加固项状态
    enum SSRReinforcementState
    {
        // 未知
        SSR_REINFORCEMENT_STATE_UNKNOWN = 0,
        // 符合标准
        SSR_REINFORCEMENT_STATE_SAFE = (1 << 0),
        // 不符合标准
        SSR_REINFORCEMENT_STATE_UNSAFE = (1 << 1),
        // 未扫描，加固项还未扫描任务就被取消则会变为该状态
        SSR_REINFORCEMENT_STATE_UNSCAN = (1 << 2),
        // 扫描中
        SSR_REINFORCEMENT_STATE_SCANNING = (1 << 3),
        // 扫描错误
        SSR_REINFORCEMENT_STATE_SCAN_ERROR = (1 << 4),
        // 扫描完成
        SSR_REINFORCEMENT_STATE_SCAN_DONE = (1 << 5),
        // 未加固，加固项还未加固任务就被取消则会变为该状态
        SSR_REINFORCEMENT_STATE_UNREINFORCE = (1 << 6),
        // 加固错误
        SSR_REINFORCEMENT_STATE_REINFORCE_ERROR = (1 << 7),
        // 加固中
        SSR_REINFORCEMENT_STATE_REINFORCING = (1 << 8),
        // 加固完成
        SSR_REINFORCEMENT_STATE_REINFORCE_DONE = (1 << 9)
    };

    // 加固类型
    enum SSRReinforceType
    {
        // 	使用默认方式加固
        SEE_REINFORCE_TYPE_DEFAULT = 0,
        // 使用自定义参数加固
        SEE_REINFORCE_TYPE_CUSTOM
    };

    // 任务状态
    enum SSRJobState
    {
        // 空闲
        SSR_JOB_STATE_IDLE,
        // 执行中
        SSR_JOB_STATE_RUNNING,
        // 执行完成
        SSR_JOB_STATE_DONE,
        // 执行完成(中途被取消)
        SSR_JOB_STATE_CANCEL_DONE,
    };

#ifdef __cplusplus
}
#endif