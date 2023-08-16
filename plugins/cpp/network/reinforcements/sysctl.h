/**
 * @file          /kiran-ssr-manager/plugins/cpp/network/reinforcements/sysctl.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "lib/config/config-plain.h"

namespace Kiran
{
namespace Network
{
class Sysctl
{
public:
    Sysctl();
    virtual ~Sysctl(){};

protected:
    using SysctlVar = std::pair<std::string, std::string>;

protected:
    std::vector<SysctlVar> get_vars_by_pattern(const std::string &pattern);

protected:
    std::shared_ptr<ConfigPlain> sysctl_config_;
};

class SysctlRedirect : public Sysctl
{
public:
    SysctlRedirect();
    virtual ~SysctlRedirect(){};

    virtual bool get(std::string &args, SSRErrorCode &error_code);
    virtual bool set(const std::string &args, SSRErrorCode &error_code);
};

class SysctlSourceRoute : public Sysctl
{
public:
    SysctlSourceRoute();
    virtual ~SysctlSourceRoute(){};

    virtual bool get(std::string &args, SSRErrorCode &error_code);
    virtual bool set(const std::string &args, SSRErrorCode &error_code);
};

}  // namespace Network

}  // namespace Kiran
