/**
 * @file          /kiran-sse-manager/plugins/cpp/config/reinforcements/login-lock.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/cpp/config/reinforcements/login-lock.h"
#include <json/json.h>
#include <unistd.h>

namespace Kiran
{
namespace Config
{

#define LOGIN_LOCK_CONF_PATH "/etc/pam.d/system-auth"
#define LOGIN_LOCK_CONF_KEY_TALLY "auth        required                                     pam_tally.so"

LoginLock::LoginLock()
{
    this->login_lock_config_ = ConfigPAM::create(LOGIN_LOCK_CONF_KEY_TALLY);
}


bool LoginLock::get(const std::string &rs, SSRErrorCode &error_code)
{
    if(!this->password_complextiy_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        auto tally = this->login_lock_config_->get_all_value(LOGIN_LOCK_CONF_KEY_TALLY);
        values[LOGIN_LOCK_CONF_KEY_TALLY] = tally;
        args = StrUtils::json2str(values);
        return true;
    }
    catch(const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
}

//XXX：有关操作数的问题，需要对有操作数的内容进行处理。
bool LoginLock::set(const std::string &ra, SSRErrorCode &error_code)
{
    if(!this->password_complextiy_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        auto keys = values.getMemberNames();
        for(auto iter : keys)
        {
            this->login_lock_config_->set_value(iter, values[iter]);
        }
        return true;
    }
    catch(const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
}

}  // namespace Config
}  // namespace Kiran