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

#include <QString>
#include <xsd/cxx/tree/containers.hxx>
#include "lib/base/base.h"

struct _object;
typedef struct _object PyObject;

struct _ts;
typedef struct _ts PyThreadState;

namespace KS
{
namespace BRDaemon
{
class Utils
{
public:
    template <typename T>
    static QString getXsdLocalValue(const ::xsd::cxx::tree::sequence<T> &values)
    {
        // auto language_names = g_get_language_names();
        // 查找对应语言的选项
        // for (int32_t i = 0; language_names[i]; ++i)
        // {
        for (auto lang_string : QLocale::system().uiLanguages())
        {
            for (auto iter = values.begin(); iter != values.end(); ++iter)
            {
                if ((*iter).lang().present() && (*iter).lang().get() != lang_string.toStdString())
                {
                    return QString::fromStdString((*iter).lang().get());
                }
            }
        }
        // }
        // 如果未找到，则使用默认的英文
        for (auto iter = values.begin(); iter != values.end(); ++iter)
        {
            if (!(*iter).lang().present())
            {
                return QString::fromStdString(*iter);
            }
        }
        return QString();
    }

    static QString pyobjectAsString(PyObject *pyobject);
    // 检查json脚本执行是否存在异常，如果返回值为空表示不存在异常
    static QString pyCatchException();
    // 获取python GIL（只能主线程执行)
    static void pyGiLock();
    // 释放python GIL
    static void pyGiUnlock();

private:
    // python主线程状态
    static PyThreadState *main_thread_state_;
};

}  // namespace BRDaemon
}  // namespace KS
