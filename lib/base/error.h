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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */
#ifndef ERROR_H
#define ERROR_H

#include <QString>
#include <cstdint>
#include "include/ssr-marcos.h"
#include "ssr-error-i.h"

namespace KS
{
#define SSR_ERROR2STR(errorCode) Error::getErrorDesc(errorCode)

class Error
{
public:
    Error();
    ~Error();

    static QString getErrorDesc(SSRErrorCode errorCode);
};

extern std::string dbus_error_message;

#define BR_ERROR2STR(error_code) BRError::getErrorDesc(error_code)

#define THROW_DBUSCXX_ERROR(error_code, ...)                                       \
    {                                                                              \
        dbus_error_message = fmt::format(BR_ERROR2STR(error_code), ##__VA_ARGS__); \
        sendErrorReply(QDBusError::InvalidMember, dbus_error_message);             \
    }

#define DBUS_ERROR_REPLY(error_code, ...)                                                    \
    {                                                                                        \
        auto err_message = fmt::format(BR_ERROR2STR(error_code), ##__VA_ARGS__);             \
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err_message.c_str())); \
    }

#define DBUS_ERROR_REPLY_AND_RET(error_code, ...) \
    DBUS_ERROR_REPLY(error_code, ##__VA_ARGS__);  \
    return;

class BRError
{
public:
    BRError();
    virtual ~BRError(){};

    static QString getErrorDesc(BRErrorCode error_code);
};

}  // namespace KS

#endif  // ERROR_H
