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
     * @brief 判断加固参数是否符合加固标准
     * @param {string} rs 加固标准
     * @param {string} ra 加固参数
     * @return {*} 如果加固参数符合加固标准，则返回true，否则返回false
     */
    virtual bool RAMatchRS(const std::string &rs, const std::string &ra) = 0;

    /**
     * @brief 判断系统配置是否符合加固标准
     * @param {string} rs 加固标准
     * @return {*} 如果系统配置符合加固标准，则返回true，否则返回false
     */
    virtual bool SCMatchRS(const std::string &rs) = 0;

    /**
     * @brief 根据加固参数进行加固
     * @param {string} ra 加固参数
     * @return {*} 返回加固结果
     */
    virtual bool Reinforce(const std::string &ra, SSEErrorCode &error_code) = 0;
};
