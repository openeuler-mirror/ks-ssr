/**
 * @file          /ks-ssr-manager/lib/base/misc-utils.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "lib/base/misc-utils.h"
#include "lib/base/base.h"

namespace KS
{
MiscUtils::MiscUtils()
{
}

bool MiscUtils::spawn_sync(const std::vector<std::string> &argv,
                           std::string *standard_output,
                           std::string *standard_error)
{
    KLOG_DEBUG("Exec command: %s.", StrUtils::join(argv, " ").c_str());

    try
    {
        int32_t exit_status = 0;
        Glib::spawn_sync(std::string(),
                         argv,
                         Glib::SpawnFlags(0),
                         sigc::slot<void>(),
                         standard_output,
                         standard_error,
                         &exit_status);

        if (exit_status)
        {
            KLOG_WARNING("Failed to exec command %s, exit status: %d.", StrUtils::join(argv, " ").c_str(), exit_status);
            return false;
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return false;
    }
    return true;
}

Glib::OptionEntry MiscUtils::create_option_entry(const char &short_name,
                                                 const Glib::ustring &long_name,
                                                 const Glib::ustring &description,
                                                 const Glib::ustring &arg_description,
                                                 int32_t flags)
{
    Glib::OptionEntry result;
    result.set_short_name(short_name);
    result.set_long_name(long_name);
    result.set_description(description);
    result.set_arg_description(arg_description);
    result.set_flags(flags);
    return result;
}

Glib::OptionEntry MiscUtils::create_option_entry(const Glib::ustring &long_name,
                                                 const Glib::ustring &description,
                                                 const Glib::ustring &arg_description,
                                                 int32_t flags)
{
    Glib::OptionEntry result;
    result.set_long_name(long_name);
    result.set_description(description);
    result.set_arg_description(arg_description);
    result.set_flags(flags);
    return result;
}
}  // namespace KS
