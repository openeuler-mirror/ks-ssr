/**
 * @file          /ks-ssr-manager/lib/base/misc-utils.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include <giomm.h>

namespace KS
{
class MiscUtils
{
public:
    MiscUtils();
    virtual ~MiscUtils(){};

    static bool spawn_sync(const std::vector<std::string> &argv,
                           std::string *standard_output = nullptr,
                           std::string *standard_error = nullptr);

    static Glib::OptionEntry create_option_entry(const char &short_name,
                                                 const Glib::ustring &long_name,
                                                 const Glib::ustring &description,
                                                 const Glib::ustring &arg_description = Glib::ustring(),
                                                 int32_t flags = 0);

    static Glib::OptionEntry create_option_entry(const Glib::ustring &long_name,
                                                 const Glib::ustring &description,
                                                 const Glib::ustring &arg_description = Glib::ustring(),
                                                 int32_t flags = 0);

};  // namespace KS

}  // namespace KS
