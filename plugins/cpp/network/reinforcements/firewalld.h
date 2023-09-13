/**
 * @file          /ks-br-manager/plugins/cpp/network/reinforcements/firewalld.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "br-plugin-i.h"
#include "lib/base/base.h"
#include "lib/dbus/dbus-proxy-systemd.h"

namespace KS
{
namespace Network
{
class FirewalldSwitch : public BRReinforcementInterface
{
public:
    FirewalldSwitch();
    virtual ~FirewalldSwitch(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);
    virtual bool set(const std::string &args, BRErrorCode &error_code);

private:
    std::shared_ptr<DBusSystemdProxy> systemd_proxy_;
};

class FirewalldICMPTimestamp : public BRReinforcementInterface
{
public:
    FirewalldICMPTimestamp();
    virtual ~FirewalldICMPTimestamp(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);
    virtual bool set(const std::string &args, BRErrorCode &error_code);
};

}  // namespace Network
}  // namespace KS
