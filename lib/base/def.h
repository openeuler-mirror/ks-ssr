/**
 * @file          /ks-ssr-manager/lib/base/def.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include <fmt/format.h>

#include <gtk3-log-i.h>
#include <cstdio>
#include <functional>

namespace KS
{
#define EPS 1e-6

#define SSR_CONNECTION(text1, text2) text1##text2
#define SSR_CONNECT(text1, text2) SSR_CONNECTION(text1, text2)
#define CONF_FILE_PERMISSION 0644

class SSRDefer
{
public:
    SSRDefer(std::function<void(std::string)> func, std::string fun_name) : func_(func),
                                                                            fun_name_(fun_name) {}
    ~SSRDefer() { func_(fun_name_); }

private:
    std::function<void(std::string)> func_;
    std::string fun_name_;
};

// helper macro for Defer class
#define SSR_SCOPE_EXIT(block) SSRDefer SSR_CONNECT(_defer_, __LINE__)([&](std::string _arg_function) block, __FUNCTION__)

#define RETURN_VAL_IF_FALSE(cond, val)             \
    {                                              \
        if (!(cond))                               \
        {                                          \
            KLOG_DEBUG("The condition is false."); \
            return val;                            \
        }                                          \
    }

#define RETURN_VAL_IF_TRUE(cond, val) \
    {                                 \
        if (cond) return val;         \
    }

#define RETURN_ERROR_IF_FALSE(cond, error_code_value) \
    {                                                 \
        if (!(cond))                                  \
        {                                             \
            KLOG_DEBUG("The condition is false.");    \
            error_code = error_code_value;            \
            return false;                             \
        }                                             \
    }

#define RETURN_ERROR_IF_TRUE(cond, error_code_value) \
    {                                                \
        if (cond)                                    \
        {                                            \
            error_code = error_code_value;           \
            KLOG_DEBUG("The condition is false.");   \
            return false;                            \
        }                                            \
    }

#define RETURN_IF_FALSE(cond)                      \
    {                                              \
        if (!(cond))                               \
        {                                          \
            KLOG_DEBUG("The condition is false."); \
            return;                                \
        }                                          \
    }

#define RETURN_IF_TRUE(cond) \
    {                        \
        if (cond) return;    \
    }

#define CONTINUE_IF_FALSE(cond) \
    {                           \
        if (!(cond)) continue;  \
    }

#define CONTINUE_IF_TRUE(cond) \
    {                          \
        if (cond) continue;    \
    }

#define IGNORE_EXCEPTION(expr)                  \
    {                                           \
        try                                     \
        {                                       \
            expr;                               \
        }                                       \
        catch (const Glib::Error &e)            \
        {                                       \
            KLOG_DEBUG("%s", e.what().c_str()); \
        }                                       \
        catch (const std::exception &e)         \
        {                                       \
            KLOG_DEBUG("%s", e.what());         \
        }                                       \
    }

#define POINTER_TO_STRING(p) ((p) ? p : std::string())

using StringHash = uint32_t;

constexpr StringHash prime = 9973;
constexpr StringHash basis = 0xCBF29CE4ul;
constexpr StringHash hash_compile_time(char const *str, StringHash last_value = basis)
{
    return *str ? hash_compile_time(str + 1, (StringHash)((*str ^ last_value) * (uint64_t)prime)) : last_value;
}

inline StringHash shash(char const *str)
{
    StringHash ret{basis};

    while (*str)
    {
        ret ^= *str;
        ret *= prime;
        str++;
    }

    return ret;
}

/// compile-time hash of string.
/// usage: "XXX"_hash
constexpr StringHash operator"" _hash(char const *p, size_t)
{
    return hash_compile_time(p);
}

}  // namespace KS
