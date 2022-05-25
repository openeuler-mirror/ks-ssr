/**
 * @file          /ks-ssr-manager/src/daemon/utils.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include <xsd/cxx/tree/containers.hxx>
#include "lib/base/base.h"

struct _object;
typedef struct _object PyObject;

struct _ts;
typedef struct _ts PyThreadState;

namespace KS
{
namespace Daemon
{
class Utils
{
public:
    template <typename T>
    static std::string get_xsd_local_value(const ::xsd::cxx::tree::sequence<T> &values)
    {
        auto language_names = g_get_language_names();
        // 查找对应语言的选项
        for (int32_t i = 0; language_names[i]; ++i)
        {
            for (auto iter = values.begin(); iter != values.end(); ++iter)
            {
                if ((*iter).lang().present() && (*iter).lang().get() == language_names[i])
                {
                    return std::string(*iter);
                }
            }
        }
        // 如果未找到，则使用默认的英文
        for (auto iter = values.begin(); iter != values.end(); ++iter)
        {
            if (!(*iter).lang().present())
            {
                return std::string(*iter);
            }
        }
        return std::string();
    }

    static std::string pyobject_as_string(PyObject *pyobject);
    // 检查json脚本执行是否存在异常，如果返回值为空表示不存在异常
    static std::string py_catch_exception();
    // 获取python GIL（只能主线程执行)
    static void py_gi_lock();
    // 释放python GIL
    static void py_gi_unlock();

private:
    // python主线程状态
    static PyThreadState *main_thread_state_;
};

}  // namespace Daemon
}  // namespace KS
