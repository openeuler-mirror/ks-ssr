/**
 * @file          /kiran-ssr-manager/lib/base/misc-utils.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include <giomm.h>

namespace Kiran
{
class MiscUtils
{
public:
    MiscUtils();
    virtual ~MiscUtils(){};

    static bool spawn_sync(const std::vector<std::string> &argv,
                           std::string *standard_output = nullptr,
                           std::string *standard_error = nullptr);

    // static std::string get_locale_string(const std::ve);
};

}  // namespace Kiran
