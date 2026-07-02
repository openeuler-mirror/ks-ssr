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

#define BREAK_IF_FALSE(cond) \
    {                        \
        if (!(cond)) break;  \
    }

#define BREAK_IF_TRUE(cond) \
    {                       \
        if (cond) break;    \
    }

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

}  // namespace KS
