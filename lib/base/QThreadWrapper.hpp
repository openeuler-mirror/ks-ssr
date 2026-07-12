/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#ifndef __KS_SSR_QT_THREAD_WRAPPER_HPP__
#define __KS_SSR_QT_THREAD_WRAPPER_HPP__

#include <QThread>
#include <functional>

template <typename T>
struct function_traits;

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(Args...)>
{
    using return_type = ReturnType;
    using function_type = ReturnType(Args...);
    using stl_function_type = std::function<function_type>;
    using pointer = ReturnType (*)(Args...);
    using f_args = std::tuple<Args...>;
};

template <typename Callable>
struct function_traits : function_traits<decltype(&Callable::operator())>
{
};

template <typename T>
struct QThreadWrapper;

template <typename ReturnType, typename... Args>
struct QThreadWrapper<ReturnType(Args...)> : public QThread
{
public:
    using Func = ReturnType(Args...);
    using stlFunc = std::function<ReturnType()>;

    /// @note if u need use QThreadWrapper with with c c_style_variadic, please use std::bind
    /// example: `QThreadWrapper<int(const char*)> wrapper(std::bind(printf, "example"));`
    QThreadWrapper(Func _f, Args... args)
        : f(std::bind(_f, args...)) {}

    QThreadWrapper(stlFunc _f)
        : f(_f) {}

    void run() override
    {
        f();
    }

    void detach()
    {
        start();
    }

private:
    stlFunc f;
};

template <typename ReturnType, typename ClassType, typename... Args>
struct QThreadWrapper<ReturnType (ClassType::*)(Args...)> : public QThread
{
public:
    using Func = ReturnType (ClassType::*)(Args...);
    using stlFunc = std::function<ReturnType()>;

    QThreadWrapper(Func _f, ClassType* _this, Args... args)
        : f(std::bind(_f, _this, args...)) {}

    void run() override
    {
        f();
    }

    void detach()
    {
        start();
    }

private:
    stlFunc f;
};

#endif