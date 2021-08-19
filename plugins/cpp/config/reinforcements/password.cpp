/**
 * @file          /kiran-sse-manager/plugins/cpp/config/reinforcements/password.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#include "plugins/cpp/config/reinforcements/password.h"
#include <json/json.h>
#include <unistd.h>

namespace Kiran
{
namespace Config
{

#define PASSWORD_EXPIRED_CONF_PATH "/etc/login.defs"
#define PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS "PASS_MAX_DAYS"
#define PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS "PASS_MIN_DAYS"
#define PASSWORD_EXPIRED_CONF_KEY_MIN_LEN "PASS_MIN_LEN"
#define PASSWORD_EXPIRED_CONF_KEY_WARN_AGE "PASS_WARN_AGE"

#define PASSWORD_COMPLEXTIY_CONF_PATH "/etc/pam.d/system-auth"
#define PASSWORD_COMPLEXTIY_CONF_KEY_PWQUALITY  "password    requisite                                    pam_pwquality.so"

PasswordExpired::PasswordExpired()
{
    this->password_expired_config_ = ConfigPlain::create(PASSWORD_EXPIRED_CONF_PATH);
}

bool PasswordExpired::get(const std::string &args,  SSRErrorCode &error_code)
{
    if(!this->password_complextiy_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        auto max_days = this->password_expired_config_->get_value(PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS);
        auto min_days = this->password_expired_config_->get_value(PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS);
        auto min_len = this->password_expired_config_->get_value(PASSWORD_EXPIRED_CONF_KEY_MIN_LEN);
        auto warn_age = this->password_expired_config_->get_value(PASSWORD_EXPIRED_CONF_KEY_WARN_AGE);

        value[PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS] =  max_days;
        value[PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS] =  min_days;
        value[PASSWORD_EXPIRED_CONF_KEY_MIN_LEN]  =  min_len;
        value[PASSWORD_EXPIRED_CONF_KEY_WARN_AGE] =  warn_age;

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

bool PasswordExpired::set(const std::string &args, SSEErrorCode &error_code)
{
    if(!this->password_expired_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);

        if(!values[PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS].asInt() || !values[PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS].asInt() || !values[PASSWORD_EXPIRED_CONF_KEY_MIN_LEN].asInt() || !values[PASSWORD_EXPIRED_CONF_KEY_WARN_AGE].asInt())
        {
            error_code = SSRErrorCode::ERROR_FAILED;
            return false;
        }

        auto max_days = fmt::format("{0}", values[PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS].asInt());
        auto min_days = fmt::format("{0}", values[PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS].asInt());
        auto min_len = fmt::format("{0}", values[PASSWORD_EXPIRED_CONF_KEY_MIN_LEN].asInt());
        auto warn_age = fmt::format("{0}", values[PASSWORD_EXPIRED_CONF_KEY_WARN_AGE].asInt());

        this->password_expired_config_->set_value(PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS, max_days);
        this->password_expired_config_->set_value(PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS, min_days);
        this->password_expired_config_->set_value(PASSWORD_EXPIRED_CONF_KEY_MIN_LEN, min_len);
        this->password_expired_config_->set_value(PASSWORD_EXPIRED_CONF_KEY_WARN_AGE, warn_age);
    }
    catch(const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

PasswordComplextiy::PasswordComplextiy()
{
    this->password_complextiy_config_ = ConfigPAM::create(PASSWORD_COMPLEXTIY_CONF_PATH);
}

//XXX：还存在问题，参数问题，接口返回的是一个参数的vector，无法转化为key-value形式处理。
bool PasswordComplextiy::get(std::string &args, SSRErrorCode &error_code)
{
    if(!this->password_complextiy_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        auto pwquality = this->password_complextiy_config_->get_all_value(PASSWORD_COMPLEXTIY_CONF_KEY_PWQUALITY);
        values[PASSWORD_COMPLEXTIY_CONF_KEY_PWQUALITY] = pwquality;
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

//TODO:有关操作数的问题， 大于 小于 等于 大于等于 小于等于 情况如何确定？是否可以添加操作数作为参数。
bool PasswordComplextiy::set(const std::string &args, SSRErrorCode &error_code)
{
    if(!this->password_complextiy_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);
        auto keys = values.getMemberNames();
        for(auto iter : keys)
        {
            this->password_complextiy_config_->set_value(iter, values[iter]);
        }
        // if(!values[PASSWORD_COMPLEXTIY_CONF_KEY_PWQUALITY].isArray())
        // {
        //     error_code = SSRErrorCode::ERROR_FAILED;
        //     return false;
        // }
    }
    catch(const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

}   // namespace Config
}  // namespace Kiran