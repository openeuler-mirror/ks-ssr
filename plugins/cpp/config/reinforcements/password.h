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

#pragma once

#include "lib/base/base.h"
#include "lib/config/config-pam.h"
#include "lib/config/config-plain.h"
#include "reinforcement-i.h"

namespace KS
{
namespace Config
{
class PasswordExpired : public BRReinforcementInterface
{
public:
    PasswordExpired(){};
    virtual ~PasswordExpired(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);
    virtual bool set(const std::string &args, BRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> password_expired_config_;
};

class PasswordComplextiy : public BRReinforcementInterface
{
public:
    PasswordComplextiy(){};
    virtual ~PasswordComplextiy(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);
    virtual bool set(const std::string &args, BRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPAM> password_complextiy_config_;
};

}  // namespace Config
}  // namespace KS