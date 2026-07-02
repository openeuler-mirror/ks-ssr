/**
 * @file          /kiran-ssr-manager/lib/base/glib2-utils.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include <giomm.h>

namespace Kiran
{
class Glib2Utils
{
public:
    Glib2Utils();
    virtual ~Glib2Utils(){};

    static bool spawn_sync(const std::vector<std::string> &argv,
                           std::string *standard_output = nullptr,
                           std::string *standard_error = nullptr);
};

}  // namespace Kiran
