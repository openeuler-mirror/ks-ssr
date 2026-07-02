/**
 * @file          /kiran-sse-manager/plugins/cpp/config/reinforcements/password.h
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "lib/base/base.h"
#include "lib/config/config-plain.h"
#include "lib/config/config-pam.h"
#include "reinforcement-i.h"

namespace Kiran
{
namespace Config
{
class PasswordExpired : public SSRReinforcementInterface
{
public:
    PasswordExpired(){};
    virtual ~PasswordExpired(){};

    virtual bool get(std::string &args, SSRErrorCode &error_code);
    virtual bool set(const std::string &args, SSRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> password_expired_config_;
};

class PasswordComplextiy : public SSRReinforcementInterface
{
public:
    PasswordComplextiy(){};
    virtual ~PasswordComplextiy(){};

    virtual bool get(std::string &args, SSRErrorCode &error_code);
    virtual bool set(const std::string &args, SSRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPAM> password_complextiy_config_;
};

}  // namespace Config
}  // namespace Kiran