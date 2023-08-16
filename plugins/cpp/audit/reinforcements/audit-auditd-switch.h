/**
 * @file          /kiran-sse-manager/plugins/cpp/audit/reinforcements/audit-auditd-switch.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "lib/dbus/dbus-systemd-proxy.h"
#include "reinforcement-i.h"

namespace Kiran
{
class AuditAuditdSwitch : public SSEReinforcementInterface
{
public:
    AuditAuditdSwitch();
    virtual ~AuditAuditdSwitch(){};

    virtual bool get(std::string &args, SSEErrorCode &error_code);
    virtual bool set(const std::string &args, SSEErrorCode &error_code);

private:
    DBusSystemdProxy systemd_proxy_;
};
}  // namespace Kiran