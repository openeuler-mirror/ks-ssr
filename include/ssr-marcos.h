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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <qt5-log-i.h>

#define CONNECTION(text1, text2) text1##text2
#define CONNECT(text1, text2) CONNECTION(text1, text2)
#define EPS 1e-6
#define BR_CONNECTION(text1, text2) text1##text2
#define BR_CONNECT(text1, text2) BR_CONNECTION(text1, text2)
#define CONF_FILE_PERMISSION 0644

class Defer
{
public:
    Defer(std::function<void(std::string)> func, std::string fun_name) : func_(func),
                                                                         fun_name_(fun_name) {}
    ~Defer() { func_(fun_name_); }

private:
    std::function<void(std::string)> func_;
    std::string fun_name_;
};

// helper macro for Defer class
#define SCOPE_EXIT(block) Defer CONNECT(_defer_, __LINE__)([&](std::string _arg_function) block, __FUNCTION__)

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

// dbus marcos
#define DBUS_ERROR_REPLY_AND_RETURN(errorCode, message)                                             \
    {                                                                                               \
        auto replyMessage = message.createErrorReply(QDBusError::Failed, SSR_ERROR2STR(errorCode)); \
        QDBusConnection::systemBus().send(replyMessage);                                            \
        return;                                                                                     \
    }

#define DBUS_ERROR_REPLY_AND_RETURN_VAL(val, errorCode, message)                                    \
    {                                                                                               \
        auto replyMessage = message.createErrorReply(QDBusError::Failed, SSR_ERROR2STR(errorCode)); \
        QDBusConnection::systemBus().send(replyMessage);                                            \
        return val;                                                                                 \
    }
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

#define POINTER_TO_STRING(p) ((p) ? p : QString())

// #define _(text) QObject::tr(text)