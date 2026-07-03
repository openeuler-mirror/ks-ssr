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

#include "plugins/cpp/audit/audit-plugin.h"
#include "lib/base/base.h"
#include "plugins/cpp/audit/reinforcements/auditd.h"
#include "plugins/cpp/audit/reinforcements/logrotate.h"

PLUGIN_EXPORT_FUNC_DEF(KS::Audit::BRPluginAudit);

namespace KS
{
#define AUDIT_REINFORCEMENT_AUDITD_SWITCH "audit-auditd-switch"
#define AUDIT_REINFORCEMENT_LOGROTATE_ROTATE "audit-logrotate-rotate"

namespace Audit
{
void BRPluginAudit::activate()
{
    this->reinforcements_ = std::map<std::string, std::shared_ptr<BRReinforcementInterface>>(
        {{AUDIT_REINFORCEMENT_AUDITD_SWITCH, std::make_shared<AuditdSwitch>()},
         {AUDIT_REINFORCEMENT_LOGROTATE_ROTATE, std::make_shared<LogrotateRotate>()}});
}

void BRPluginAudit::deactivate()
{
    this->reinforcements_.clear();
}

}  // namespace Audit
}  // namespace KS
