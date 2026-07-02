/**
 * @file          /ks-ssr-manager/plugins/cpp/config/reinforcements/external-login-restrictions.h
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "lib/base/base.h"
#include "lib/config/config-plain.h"
#include "reinforcement-i.h"

namespace KS
{
namespace External
{
class LoginRestrictions : public SSRReinforcementInterface
{
public:
    LoginRestrictions(){};
    virtual ~LoginRestrictions(){};

    virtual bool get(std::string &args, SSRErrorCode &error_code);
    virtual bool set(const std::string &args, SSRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> login_restrictions_config_;
};

class LoginTimeout : public SSRReinforcementInterface
{
public:
    LoginTimeout(){};
    virtual ~LoginTimeout(){};

    virtual bool get(std::string &args, SSRErrorCode &error_code);
    virtual bool set(const std::string &args, SSRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> login_timeout_config_;
} ；

}  // namespace External
}  // namespace KS