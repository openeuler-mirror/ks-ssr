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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#pragma once

#include <ssr-error-i.h>
#include <QSharedPointer>
#include <QString>
#include <memory>
#include <string>

namespace KS
{
/*
RS: reinforcements standard
RA: reinforcements arguments
SC: system configuration
*/

#if defined(WIN32) || defined(_WIN64)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

// 跟插件交互的协议ID
enum class BRPluginProtocol
{
    // 未知
    BR_PLUGIN_PROTOCOL_UNKNOWN = 0,
    // 获取系统配置
    BR_PLUGIN_PROTOCOL_GET_REQ,
    BR_PLUGIN_PROTOCOL_GET_RSP,
    // 根据加固配置进行加固
    BR_PLUGIN_PROTOCOL_SET_REQ,
    BR_PLUGIN_PROTOCOL_SET_RSP,
    BR_PLUGIN_PROTOCOL_LAST,
};

class DLLEXPORT BRReinforcementInterface
{
public:
    /**
     * @brief 获取系统配置
     * @param {string} args 系统配置参数
     * @param {string} error 如果出错则返回错误字符串
     * @return {*} 如果系统配置符合加固标准，则返回true，否则返回false
     */
    virtual bool get(QString &args, QString &error) = 0;

    /**
     * @brief 设置系统配置
     * @param {string} args 系统配置参数
     * @param {string} error 如果出错则返回错误字符串
     * @return {*} 返回加固结果
     */
    virtual bool set(const QString &args, QString &error) = 0;
};

class DLLEXPORT BRPluginInterface
{
public:
    /**
     * @brief 激活插件，可在此函数中对插件进行初始化操作
     * @return {} 无返回值
     */
    virtual void activate() = 0;

    /**
     * @brief 取消插件激活，可在此函数中释放所有对象
     * @return {} 无返回值
     */
    virtual void deactivate() = 0;

    /**
     * @brief 获取加固项
     * @param {string} 加固项的名称
     * @return {string} 加固项对象
     */
    virtual QSharedPointer<BRReinforcementInterface> getReinforcement(const QString &name) = 0;
};

typedef void *(*NewPluginFun)(void);
typedef void (*DelPluginFun)(void *);

}  // namespace KS

#define PLUGIN_EXPORT_FUNC_DEF(plugin_name)               \
    extern "C" DLLEXPORT void *new_plugin()               \
    {                                                     \
        return new plugin_name();                         \
    }                                                     \
    extern "C" DLLEXPORT void delete_plugin(void *plugin) \
    {                                                     \
        delete (plugin_name *)plugin;                     \
    }
