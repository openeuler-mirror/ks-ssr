/**
 * @file          /ks-br-manager/plugins/cpp/config/reinforcements/login-lock.h
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "lib/config/config-pam.h"
#include "reinforcement-i.h"

namespace KS
{
namespace Config
{
class LoginLock : public BRReinforcementInterface
{
public:
    LoginLock(){};
    virtual ~LoginLock(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);
    virtual bool set(const std::string &args, BRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPAM> login_lock_config_;
};

}  // namespace Config
}  // namespace KS