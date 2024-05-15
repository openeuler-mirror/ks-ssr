/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#include "plugins/cpp/config/reinforcements/password.h"
#include <json/json.h>
#include <unistd.h>

namespace KS
{
namespace Config
{
#define PASSWORD_EXPIRED_CONF_PATH "/etc/login.defs"
#define PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS "PASS_MAX_DAYS"
#define PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS "PASS_MIN_DAYS"
#define PASSWORD_EXPIRED_CONF_KEY_MIN_LEN "PASS_MIN_LEN"
#define PASSWORD_EXPIRED_CONF_KEY_WARN_AGE "PASS_WARN_AGE"

#define PASSWORD_COMPLEXTIY_CONF_PATH "/etc/pam.d/system-auth"
#define PASSWORD_COMPLEXTIY_CONF_KEY_PWQUALITY "password    requisite                                    pam_pwquality.so"

PasswordExpired::PasswordExpired()
{
    this->password_expired_config_ = ConfigPlain::create(PASSWORD_EXPIRED_CONF_PATH);
}

bool PasswordExpired::get(const std::string &args, BRErrorCode &error_code)
{
    if (!this->password_complextiy_config_)
    {
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        auto max_days = this->password_expired_config_->get_integer(PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS);
        auto min_days = this->password_expired_config_->get_integer(PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS);
        auto min_len = this->password_expired_config_->get_integer(PASSWORD_EXPIRED_CONF_KEY_MIN_LEN);
        auto warn_age = this->password_expired_config_->get_integer(PASSWORD_EXPIRED_CONF_KEY_WARN_AGE);

        value[PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS] = max_days;
        value[PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS] = min_days;
        value[PASSWORD_EXPIRED_CONF_KEY_MIN_LEN] = min_len;
        value[PASSWORD_EXPIRED_CONF_KEY_WARN_AGE] = warn_age;

        args = StrUtils::json2str(values);

        return true;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
}

bool PasswordExpired::set(const std::string &args, BRErrorCode &error_code)
{
    if (!this->password_expired_config_)
    {
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);
        RETURN_ERROR_IF_FALSE((values[PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS].isInt() ||
                               values[PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS].isInt() ||
                               values[PASSWORD_EXPIRED_CONF_KEY_MIN_LEN].isInt() ||
                               values[PASSWORD_EXPIRED_CONF_KEY_WARN_AGE].isInt()),
                              BRErrorCode::ERROR_FAILED);

        auto max_days = fmt::format("{0}", values[PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS].asInt());
        auto min_days = fmt::format("{0}", values[PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS].asInt());
        auto min_len = fmt::format("{0}", values[PASSWORD_EXPIRED_CONF_KEY_MIN_LEN].asInt());
        auto warn_age = fmt::format("{0}", values[PASSWORD_EXPIRED_CONF_KEY_WARN_AGE].asInt());

        this->password_expired_config_->set_value(PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS, max_days);
        this->password_expired_config_->set_value(PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS, min_days);
        this->password_expired_config_->set_value(PASSWORD_EXPIRED_CONF_KEY_MIN_LEN, min_len);
        this->password_expired_config_->set_value(PASSWORD_EXPIRED_CONF_KEY_WARN_AGE, warn_age);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

PasswordComplextiy::PasswordComplextiy()
{
    this->password_complextiy_config_ = ConfigPAM::create(PASSWORD_COMPLEXTIY_CONF_PATH);
}

// XXX：还存在问题，参数问题，接口返回的是一个参数的vector，无法转化为key-value形式处理。
bool PasswordComplextiy::get(std::string &args, BRErrorCode &error_code)
{
    if (!this->password_complextiy_config_)
    {
        error_code = BRErrorCode::ERROR_FAILED;
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
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
}

// TODO:有关操作数的问题， 大于 小于 等于 大于等于 小于等于 情况如何确定？是否可以添加操作数作为参数。
bool PasswordComplextiy::set(const std::string &args, BRErrorCode &error_code)
{
    if (!this->password_complextiy_config_)
    {
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);
        auto keys = values.getMemberNames();
        for (auto iter : keys)
        {
            this->password_complextiy_config_->set_value(iter, values[iter]);
        }
        // if(!values[PASSWORD_COMPLEXTIY_CONF_KEY_PWQUALITY].isArray())
        // {
        //     error_code = BRErrorCode::ERROR_FAILED;
        //     return false;
        // }
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

}  // namespace Config
}  // namespace KS