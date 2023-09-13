/**
 * @file          /ks-br-manager/plugins/cpp/audit/reinforcements/auditd.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "lib/dbus/dbus-proxy-systemd.h"

namespace KS
{
namespace Audit
{
class AuditdSwitch : public BRReinforcementInterface
{
public:
    AuditdSwitch();
    virtual ~AuditdSwitch(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);
    virtual bool set(const std::string &args, BRErrorCode &error_code);

private:
    std::shared_ptr<DBusSystemdProxy> systemd_proxy_;
};
}  // namespace Audit
}  // namespace KS