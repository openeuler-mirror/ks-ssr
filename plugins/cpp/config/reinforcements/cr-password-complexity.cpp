/**
 * @file          /kiran-sse-manager/plugins/cpp/config/reinforcements/cr-password-complexity.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "plugins/cpp/config/reinforcements/cr-password-complexity.h"
#include <json/json.h>
#include <unistd.h>

namespace Kiran
{
bool CRPasswordComplextiy::RAMatchRS(const std::string &rs, const std::string &ra)
{
    return true;
}

bool CRPasswordComplextiy::SCMatchRS(const std::string &rs)
{
    return true;
}

bool CRPasswordComplextiy::Reinforce(const std::string &ra, SSEErrorCode &error_code)
{
    sleep(10);
    return true;
}

}  // namespace Kiran