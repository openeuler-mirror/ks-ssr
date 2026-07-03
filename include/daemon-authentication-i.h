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
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#pragma once

#include <ssr-i.h>
#include <QDBusMessage>
#include <functional>

namespace KS
{
class IDaemonAuthentication;
extern IDaemonAuthentication *g_daemonAuthentication;

#define CHECK_AUTH(className, funName, callback, action, validateRole)                                            \
    void className::funName()                                                                                     \
    {                                                                                                             \
        this->setDelayedReply(true);                                                                              \
        g_daemonAuthentication->checkAuthorization(action,                                                        \
                                                   validateRole,                                                  \
                                                   this->message(),                                               \
                                                   std::bind(&className::callback, this, std::placeholders::_1)); \
    }

#define CHECK_AUTH_WITH_1ARGS(className, funName, callback, action, validateRole, arg1Type)                               \
    void className::funName(arg1Type value1)                                                                              \
    {                                                                                                                     \
        this->setDelayedReply(true);                                                                                      \
        g_daemonAuthentication->checkAuthorization(action,                                                                \
                                                   validateRole,                                                          \
                                                   this->message(),                                                       \
                                                   std::bind(&className::callback, this, std::placeholders::_1, value1)); \
    }

#define CHECK_AUTH_WITH_2ARGS(className, funName, callback, action, validateRole, arg1Type, arg2Type)                             \
    void className::funName(arg1Type value1, arg2Type value2)                                                                     \
    {                                                                                                                             \
        this->setDelayedReply(true);                                                                                              \
        g_daemonAuthentication->checkAuthorization(action,                                                                        \
                                                   validateRole,                                                                  \
                                                   this->message(),                                                               \
                                                   std::bind(&className::callback, this, std::placeholders::_1, value1, value2)); \
    }

#define CHECK_AUTH_WITH_3ARGS(className, funName, callback, action, validateRole, arg1Type, arg2Type, arg3Type)                           \
    void className::funName(arg1Type value1, arg2Type value2, arg3Type value3)                                                            \
    {                                                                                                                                     \
        this->setDelayedReply(true);                                                                                                      \
        g_daemonAuthentication->checkAuthorization(action,                                                                                \
                                                   validateRole,                                                                          \
                                                   this->message(),                                                                       \
                                                   std::bind(&className::callback, this, std::placeholders::_1, value1, value2, value3)); \
    }

#define CHECK_AUTH_WITH_1ARGS_AND_RETVAL(className, retType, funName, callback, validateRole, action, arg1Type)           \
    retType className::funName(arg1Type value1)                                                                           \
    {                                                                                                                     \
        this->setDelayedReply(true);                                                                                      \
        g_daemonAuthentication->checkAuthorization(action,                                                                \
                                                   validateRole,                                                          \
                                                   this->message(),                                                       \
                                                   std::bind(&className::callback, this, std::placeholders::_1, value1)); \
        return retType();                                                                                                 \
    }

#define CHECK_AUTH_WITH_3ARGS_AND_RETVAL(className, retType, funName, callback, validateRole, action, arg1Type, arg2Type, arg3Type)       \
    retType className::funName(arg1Type value1, arg2Type value2, arg3Type value3)                                                         \
    {                                                                                                                                     \
        this->setDelayedReply(true);                                                                                                      \
        g_daemonAuthentication->checkAuthorization(action,                                                                                \
                                                   validateRole,                                                                          \
                                                   this->message(),                                                                       \
                                                   std::bind(&className::callback, this, std::placeholders::_1, value1, value2, value3)); \
        return retType();                                                                                                                 \
    }

using checkAuthHandler = std::function<void(const QDBusMessage &)>;

class IDaemonAuthentication
{
public:
    virtual void checkAuthorization(const QString &action,
                                    const QVector<AccountRole> &validateRole,
                                    const QDBusMessage &message,
                                    checkAuthHandler handler) = 0;
};
}  // namespace  KS
