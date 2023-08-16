/**
 * @file          /kiran-ssr-manager/include/plugin-i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <string>

namespace Kiran
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
enum SSRPluginProtocol
{
    // 未知
    SSR_PLUGIN_PROTOCOL_UNKNOWN = 0,
    // 获取系统配置
    SSR_PLUGIN_PROTOCOL_GET_REQ,
    SSR_PLUGIN_PROTOCOL_GET_RSP,
    // 根据加固配置进行加固
    SSR_PLUGIN_PROTOCOL_SET_REQ,
    SSR_PLUGIN_PROTOCOL_SET_RSP,
    SSR_PLUGIN_PROTOCOL_LAST,
};

class DLLEXPORT SSRPluginInterface
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
     * @brief 执行一个操作
     * @param {string} 操作的输入，json字符串表示
     * @return {string} 操作的数处，json字符串表示
     */
    virtual std::string execute(const std::string &in_json) = 0;
};

using NewPluginFun = void *(*)(void);
using DelPluginFun = void (*)(void *);

}  // namespace Kiran

#define PLUGIN_EXPORT_FUNC_DEF(plugin_name)               \
    extern "C" DLLEXPORT void *new_plugin()               \
    {                                                     \
        return new plugin_name();                         \
    }                                                     \
    extern "C" DLLEXPORT void delete_plugin(void *plugin) \
    {                                                     \
        delete (plugin_name *)plugin;                     \
    }
