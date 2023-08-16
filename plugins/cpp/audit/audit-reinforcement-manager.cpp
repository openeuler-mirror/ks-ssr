/**
 * @file          /kiran-sse-manager/plugins/cpp/audit/audit-reinforcement-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#include "plugins/cpp/audit/audit-reinforcement-manager.h"
#include "plugins/cpp/audit/reinforcements/audit-auditd.h"
#include "plugins/cpp/audit/reinforcements/audit-logrotate.h"

namespace Kiran
{
#define AUDIT_REINFORCEMENT_AUDITD_SWITCH "audit-auditd-switch"
#define AUDIT_REINFORCEMENT_LOGROTATE_ROTATE "audit-logrotate-rotate"

AuditReinforcementManager::AuditReinforcementManager()
{
}

AuditReinforcementManager::~AuditReinforcementManager()
{
}

AuditReinforcementManager* AuditReinforcementManager::instance_ = nullptr;
void AuditReinforcementManager::global_init()
{
    instance_ = new AuditReinforcementManager();
    instance_->init();
}

void AuditReinforcementManager::init()
{
    this->reinforcements_.emplace(AUDIT_REINFORCEMENT_AUDITD_SWITCH, std::make_shared<AuditAuditdSwitch>());
    this->reinforcements_.emplace(AUDIT_REINFORCEMENT_LOGROTATE_ROTATE, std::make_shared<AuditLogrotateRotate>());
}
}  // namespace Kiran