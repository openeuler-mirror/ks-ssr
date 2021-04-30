/**
 * @file          /kiran-sse-manager/include/sse_i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define SSE_DBUS_NAME "com.kylinsec.Kiran.SSE"
#define SSE_DBUS_OBJECT_PATH "/com/kylinsec/Kiran/SSE"
#define SSE_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SSE"

    // 加固标准类型
    enum SSEStandardType
    {
        // 系统默认标准
        SSE_STANDARD_TYPE_SYSTEM = 0,
        // 用户导入的标准（自定义加固标准）
        SEE_STANDARD_TYPE_CUSTOM,
        SEE_STANDARD_TYPE_LAST,
    };

    // 加固项状态
    enum SSEReinforcementState
    {
        // 未知
        SSE_REINFORCEMENT_STATE_UNKNOWN = 0,
        // 符合标准
        SSE_REINFORCEMENT_STATE_SAFE = (1 << 0),
        // 不符合标准
        SSE_REINFORCEMENT_STATE_UNSAFE = (1 << 1),
        // 未扫描
        SSE_REINFORCEMENT_STATE_UNSCAN = (1 << 2),
        // 扫描中
        SSE_REINFORCEMENT_STATE_SCANNING = (1 << 3),
        // 扫描完成
        SSE_REINFORCEMENT_STATE_SCAN_DONE = (1 << 4),
        // 未加固
        SSE_REINFORCEMENT_STATE_UNREINFORCE = (1 << 5),
        // 加固中
        SSE_REINFORCEMENT_STATE_REINFORCING = (1 << 6),
        // 加固完成
        SSE_REINFORCEMENT_STATE_REINFORCE_DONE = (1 << 7)
    };

    // 加固类型
    enum SSEReinforceType
    {
        // 	使用默认方式加固
        SEE_REINFORCE_TYPE_DEFAULT = 0,
        // 使用自定义参数加固
        SEE_REINFORCE_TYPE_CUSTOM
    };

    // 任务状态
    enum SSEJobState
    {
        // 空闲
        SSE_JOB_STATE_IDLE,
        // 执行中
        SSE_JOB_STATE_RUNNING,
        // 执行完成
        SSE_JOB_STATE_DONE,
        // 执行完成(中途被取消)
        SSE_JOB_STATE_CANCEL_DONE,
    };

#ifdef __cplusplus
}
#endif