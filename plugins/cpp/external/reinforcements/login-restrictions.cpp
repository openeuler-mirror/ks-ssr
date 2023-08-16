/**
 * @file          /kiran-sse-manager/plugins/cpp/config/reinforcements/login-restrictions.cpp
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#include "plugins/cpp/external/reinforcements/login-restrictions.h"
#include <json/json.h>
#include <unistd.h>

namespace Kiran
{
namespace External
{

#define LOGIN_RESTRICTIONS_CONF_PATH "/etc/ssh/ssh_config"
#define LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN "PermitRootLogin"

LoginRestrictions::LoginRestrictions()
{
    this->login_restrictions_config_ = ConfigPlain::create(LOGIN_RESTRICTIONS_CONF_PATH);
}

bool LoginRestrictions::get(const std::string &args,  SSRErrorCode &error_code)
{
    if(!this->login_restrictions_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        auto root_login = this->login_restrictions_config_->get_value(LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN);
        value[LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN] =  root_login;
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

bool LoginRestrictions::set(const std::string &args, SSEErrorCode &error_code)
{
    if(!this->login_restrictions_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);

        if(!values[LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN].asString())
        {
            error_code = SSRErrorCode::ERROR_FAILED;
            return false;
        }

        auto root_login = fmt::format("{0}", values[LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN].asString());
        this->login_restrictions_config_->set_value(LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN, root_login);
    }
    catch(const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

}  // namespace External
}  // namespace Kiran