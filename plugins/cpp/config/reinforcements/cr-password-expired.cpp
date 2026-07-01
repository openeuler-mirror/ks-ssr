/**
 * @file          /kiran-sse-manager/plugins/cpp/config/reinforcements/cr-password-expired.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#include "plugins/cpp/config/reinforcements/cr-password-expired.h"
#include <json/json.h>
#include <unistd.h>

namespace Kiran
{
bool CRPasswordExpired::RAMatchRS(const std::string &rs, const std::string &ra)
{
    return true;
}

bool CRPasswordExpired::SCMatchRS(const std::string &rs)
{
    return true;
}

bool CRPasswordExpired::Reinforce(const std::string &ra, SSEErrorCode &error_code)
{
    sleep(10);
    return true;
}

}  // namespace Kiran