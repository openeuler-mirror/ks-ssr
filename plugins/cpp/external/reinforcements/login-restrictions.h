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
#include "lib/config/config-plain.h"
#include "reinforcement-i.h"

namespace KS
{
namespace External
{
class LoginRestrictions : public BRReinforcementInterface
{
public:
    LoginRestrictions(){};
    virtual ~LoginRestrictions(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);
    virtual bool set(const std::string &args, BRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> login_restrictions_config_;
};

class LoginTimeout : public BRReinforcementInterface
{
public:
    LoginTimeout(){};
    virtual ~LoginTimeout(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);
    virtual bool set(const std::string &args, BRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> login_timeout_config_;
} ；

}  // namespace External
}  // namespace KS