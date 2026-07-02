/**
 * @file          /kiran-ssr-manager/lib/base/glib2-utils.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "lib/base/glib2-utils.h"
#include "lib/base/base.h"

namespace Kiran
{
Glib2Utils::Glib2Utils()
{
}

bool Glib2Utils::spawn_sync(const std::vector<std::string> &argv,
                            std::string *standard_output ,
                            std::string *standard_error)
{
    try
    {
        int32_t exit_status = 0;
        Glib::spawn_sync(std::string(),
                         argv,
                         Glib::SPAWN_DEFAULT,
                         Glib::SlotSpawnChildSetup(),
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
}  // namespace Kiran
