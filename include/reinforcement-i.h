/**
 * @file          /kiran-sse-manager/include/reinforcement-i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <error-i.h>
#include <string>

class SSEReinforcementInterface
{
public:
    /**
     * @brief 获取系统配置
     * @param {string} args 系统配置参数
     * @return {*} 如果系统配置符合加固标准，则返回true，否则返回false
     */
    virtual bool get(std::string &args, SSEErrorCode &error_code) = 0;

    /**
     * @brief 设置系统配置
     * @param {string} args 系统配置参数
     * @return {*} 返回加固结果
     */
    virtual bool set(const std::string &args, SSEErrorCode &error_code) = 0;
};
