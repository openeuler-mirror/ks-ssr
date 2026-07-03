/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * ks-sc is licensed under Mulan PSL v2.
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
        auto replyMessage = message.createErrorReply(QDBusError::Failed, KSC_ERROR2STR(errorCode)); \
        QDBusConnection::systemBus().send(replyMessage);                                            \
        return;                                                                                     \
    }

#define DBUS_ERROR_REPLY_AND_RETURN_VAL(val, errorCode, message)                                    \
    {                                                                                               \
        auto replyMessage = message.createErrorReply(QDBusError::Failed, KSC_ERROR2STR(errorCode)); \
        QDBusConnection::systemBus().send(replyMessage);                                            \
        return val;                                                                                 \
    }
