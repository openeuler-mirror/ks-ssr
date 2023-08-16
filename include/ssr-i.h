/**
 * @file          /kiran-ssr-manager/include/ssr-i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

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
        // 未扫描
        SSR_REINFORCEMENT_STATE_UNSCAN = (1 << 2),
        // 扫描中
        SSR_REINFORCEMENT_STATE_SCANNING = (1 << 3),
        // 扫描完成
        SSR_REINFORCEMENT_STATE_SCAN_DONE = (1 << 4),
        // 未加固
        SSR_REINFORCEMENT_STATE_UNREINFORCE = (1 << 5),
        // 加固中
        SSR_REINFORCEMENT_STATE_REINFORCING = (1 << 6),
        // 加固完成
        SSR_REINFORCEMENT_STATE_REINFORCE_DONE = (1 << 7)
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

    /* 加固配置格式：
    {
        "head": 
        {
            "name": ""          # 名字
            "version": ""       # 版本
        },
        "body": 
        {
            "item_count": 1,    # 加固项数量
            "items": 
            [
                {
                    "name": "config-login-lock",  # 加固项名称
                    "category_name" : "xxxx",     # 加固项的分类名
                    "label": "",                  # 加固项标签
                    "layout": "",                 # 加固项UI布局
                    "rules"                       # 加固标准
                    {                   
                        "deny": 
                        {
                            "type": 1,
                            "min_value" : 1,
                            "max_value" : 6
                        },
                        "unlock_time": 
                        {
                            "type": 1,
                            "min_value" : 30,
                            "max_value" : 1800
                        }
                    },
                    "default_args":               # 默认加固参数
                    {            
                        "deny": 3,
                        "unlock_time": 300
                    },
                    "custom_args":                # 自定义加固参数
                    {
                        "deny": 4,
                        "unlock_time": 400
                    }
                }
            ]
        }
    }*/

#define SSR_JSON_HEAD "head"
#define SSR_JSON_HEAD_NAME "name"
#define SSR_JSON_HEAD_VERSION "version"
#define SSR_JSON_HEAD_ERROR_CODE "error_code"
// 调用插件接口时的协议ID
#define SSR_JSON_HEAD_PROTOCOL_ID "id"

#define SSR_JSON_BODY "body"
#define SSR_JSON_BODY_MATCH "match"
// 加固项的数量
#define SSR_JSON_BODY_REINFORCEMENT_COUNT "item_count"
// 加固项
#define SSR_JSON_ITEMS "items"
#define SSR_JSON_BODY_ITEMS "items"
// 加固项信息
#define SSR_JSON_BODY_REINFORCEMENT_NAME "name"
#define SSR_JSON_BODY_REINFORCEMENT_CATEGORY_NAME "category_name"
#define SSR_JSON_BODY_REINFORCEMENT_LABEL "label"
#define SSR_JSON_BODY_REINFORCEMENT_LAYOUT "layout"
// 加固项的加固规则
#define SSR_JSON_BODY_RULES "rules"
#define SSR_JSON_BODY_RULES_TYPE "type"
#define SSR_JSON_BODY_RULES_MIN_VALUE "min_value"
#define SSR_JSON_BODY_RULES_MAX_VALUE "max_value"
// 加固项的加固参数。如果加固项存在自定义参数，则使用自定义参数，否则使用默认参数
#define SSR_JSON_BODY_REINFORCEMENT_ARGS "args"
// 加固项默认参数
#define SSR_JSON_BODY_REINFORCEMENT_DEFAULT_ARGS "default_args"
// 加固项自定义参数
#define SSR_JSON_BODY_REINFORCEMENT_CUSTOM_ARGS "custom_args"
// 系统配置参数
#define SSR_JSON_BODY_REINFORCEMENT_SYSTEM_ARGS "system_args"

    /* 扫描JSON格式：
    {
        "items":
        [
            {
                "name" : "xxxx"
            },
            {
                "name" : "yyyy"
            }
        ]
    }

    扫描结果JSON格式：
    {
        "process" : 50,
        "job_id" : 1,
        "state" : SSR_JOB_STATE_RUNNING,
        "items":
        [
            {
                "name" : "xxxx",
                "state": SSR_REINFORCEMENT_STATE_SCANNING
            },
            {
                "name" : "yyyy",
                "state": SSR_REINFORCEMENT_STATE_SCANNING
            }
        ]
    }
    */

#define SSR_JSON_SCAN_ITEMS "items"
#define SSR_JSON_SCAN_REINFORCEMENT_NAME "name"
#define SSR_JSON_SCAN_REINFORCEMENT_STATE "state"
#define SSR_JSON_SCAN_PROCESS "process"
#define SSR_JSON_SCAN_JOB_ID "job_id"
#define SSR_JSON_SCAN_JOB_STATE "job_state"

#ifdef __cplusplus
}
#endif