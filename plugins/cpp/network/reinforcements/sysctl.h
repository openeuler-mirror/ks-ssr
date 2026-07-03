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

namespace KS
{
namespace Network
{
class Sysctl
{
public:
    Sysctl();
    virtual ~Sysctl(){};

protected:
    typedef std::pair<std::string, std::string> SysctlVar;

protected:
    std::vector<SysctlVar> get_vars_by_pattern(const std::string &pattern);

protected:
    std::shared_ptr<ConfigPlain> sysctl_config_;
};

class SysctlRedirect : public Sysctl, public BRReinforcementInterface
{
public:
    SysctlRedirect();
    virtual ~SysctlRedirect(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);
    virtual bool set(const std::string &args, BRErrorCode &error_code);
};

class SysctlSourceRoute : public Sysctl, public BRReinforcementInterface
{
public:
    SysctlSourceRoute();
    virtual ~SysctlSourceRoute(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);
    virtual bool set(const std::string &args, BRErrorCode &error_code);
};

}  // namespace Network

}  // namespace KS
