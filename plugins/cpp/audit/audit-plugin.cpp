/**
 * @file          /ks-ssr-manager/plugins/cpp/audit/audit-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/cpp/audit/audit-plugin.h"
#include "lib/base/base.h"
#include "plugins/cpp/audit/reinforcements/auditd.h"
#include "plugins/cpp/audit/reinforcements/logrotate.h"

PLUGIN_EXPORT_FUNC_DEF(KS::Audit::SSRPluginAudit);

namespace KS
{
#define AUDIT_REINFORCEMENT_AUDITD_SWITCH "audit-auditd-switch"
#define AUDIT_REINFORCEMENT_LOGROTATE_ROTATE "audit-logrotate-rotate"

namespace Audit
{
void SSRPluginAudit::activate()
{
    this->reinforcements_ = std::map<std::string, std::shared_ptr<SSRReinforcementInterface>>(
        {{AUDIT_REINFORCEMENT_AUDITD_SWITCH, std::make_shared<AuditdSwitch>()},
         {AUDIT_REINFORCEMENT_LOGROTATE_ROTATE, std::make_shared<LogrotateRotate>()}});
}

void SSRPluginAudit::deactivate()
{
    this->reinforcements_.clear();
}

}  // namespace Audit
}  // namespace KS
