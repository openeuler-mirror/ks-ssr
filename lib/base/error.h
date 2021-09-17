/**
 * @file          /kiran-ssr-manager/lib/base/error.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once
#include <giomm.h>

#include <cstdint>

#include "ssr-error-i.h"

namespace Kiran
{
#define CC_ERROR2STR(error_code) SSRError::get_error_desc(error_code)

#define DBUS_ERROR_REPLY(error_code, ...)                                                    \
    {                                                                                        \
        auto err_message = fmt::format(CC_ERROR2STR(error_code), ##__VA_ARGS__);             \
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err_message.c_str())); \
    }

#define DBUS_ERROR_REPLY_AND_RET(error_code, ...) \
    DBUS_ERROR_REPLY(error_code, ##__VA_ARGS__);  \
    return;

class SSRError
{
public:
    SSRError();
    virtual ~SSRError(){};

    static std::string get_error_desc(SSRErrorCode error_code);
};

}  // namespace Kiran