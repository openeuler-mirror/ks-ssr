/**
 * @file          /ks-ssr-manager/plugins/cpp/config/reinforcements/login-restrictions.cpp
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#include "plugins/cpp/external/reinforcements/login-restrictions.h"
#include <json/json.h>
#include <unistd.h>

namespace KS
{
namespace External
{
#define LOGIN_RESTRICTIONS_CONF_PATH "/etc/ssh/ssh_config"
#define LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN "PermitRootLogin"

#define LOGIN_TIMEOUT_CONF_PATH "/etc/profile"
#define LOGIN_TIMEOUT_CONF_KEY_TMOUT "TMOUT"

LoginRestrictions::LoginRestrictions()
{
    this->login_restrictions_config_ = ConfigPlain::create(LOGIN_RESTRICTIONS_CONF_PATH);
}

bool LoginRestrictions::get(const std::string &args, SSRErrorCode &error_code)
{
    if (!this->login_restrictions_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        auto root_login = this->login_restrictions_config_->get_value(LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN);
        value[LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN] = root_login;
        args = StrUtils::json2str(values);

        return true;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
}

bool LoginRestrictions::set(const std::string &args, SSRErrorCode &error_code)
{
    if (!this->login_restrictions_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);
        RETURN_ERROR_IF_FALSE(values[LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN].isString(), SSRErrorCode::ERROR_FAILED);

        auto root_login = fmt::format("{0}", values[LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN].asString());
        this->login_restrictions_config_->set_value(LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN, root_login);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

LoginTimeout::LoginTimeout()
{
    this->login_timeout_config_ = ConfigPlain::create(LOGIN_TIMEOUT_CONF_PATH, "=");
}

bool LoginTimeout::get(const std::string &args, SSRErrorCode &error_code)
{
    if (!this->login_timeout_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        auto timeout = this->login_timeout_config_->get_value(LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN);
        value[LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN] = timeout;
        args = StrUtils::json2str(values);

        return true;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
}

bool LoginTimeout::set(const std::string &args, SSRErrorCode &error_code)
{
    if (!this->login_timeout_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);
        RETURN_ERROR_IF_FALSE(values[LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN].isInt(), SSRErrorCode::ERROR_FAILED);

        auto timeout = fmt::format("{0}", values[LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN].asInt());
        this->login_timeout_config_->set_value(LOGIN_RESTRICTIONS_CONF_KEY_ROOTLOGIN, timeout);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

}  // namespace External
}  // namespace KS