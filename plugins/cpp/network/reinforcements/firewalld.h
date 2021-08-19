/**
 * @file          /kiran-ssr-manager/plugins/cpp/network/reinforcements/firewalld.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "lib/dbus/dbus-proxy-systemd.h"
#include "reinforcement-i.h"

namespace Kiran
{
namespace Network
{
class FirewalldSwitch : public SSRReinforcementInterface
{
public:
    FirewalldSwitch();
    virtual ~FirewalldSwitch(){};

    virtual bool get(std::string &args, SSRErrorCode &error_code);
    virtual bool set(const std::string &args, SSRErrorCode &error_code);

private:
    std::shared_ptr<DBusSystemdProxy> systemd_proxy_;
};

class FirewalldICMPTimestamp : public SSRReinforcementInterface
{
public:
    FirewalldICMPTimestamp();
    virtual ~FirewalldICMPTimestamp(){};

    virtual bool get(std::string &args, SSRErrorCode &error_code);
    virtual bool set(const std::string &args, SSRErrorCode &error_code);
};

}  // namespace Network
}  // namespace Kiran
