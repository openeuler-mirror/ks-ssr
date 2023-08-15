/**
 * @file          /kiran-sse-manager/plugins/cpp/config/reinforcements/cr-login-lock.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/cpp/config/reinforcements/cr-login-lock.h"
#include <json/json.h>
#include <unistd.h>

namespace Kiran
{
bool CRLoginLock::RAMatchRS(const std::string &rs, const std::string &ra)
{
    return true;
}

bool CRLoginLock::SCMatchRS(const std::string &rs)
{
    return true;
}

bool CRLoginLock::Reinforce(const std::string &ra, SSEErrorCode &error_code)
{
    sleep(15);
    return true;
}

}  // namespace Kiran