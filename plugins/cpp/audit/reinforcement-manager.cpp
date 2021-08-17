/**
 * @file          /kiran-ssr-manager/plugins/cpp/audit/reinforcement-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "plugins/cpp/audit/reinforcement-manager.h"
#include "plugins/cpp/audit/reinforcements/auditd.h"
#include "plugins/cpp/audit/reinforcements/logrotate.h"

namespace Kiran
{
namespace Audit
{
#define AUDIT_REINFORCEMENT_AUDITD_SWITCH "audit-auditd-switch"
#define AUDIT_REINFORCEMENT_LOGROTATE_ROTATE "audit-logrotate-rotate"

ReinforcementManager::ReinforcementManager()
{
}

ReinforcementManager::~ReinforcementManager()
{
}

ReinforcementManager* ReinforcementManager::instance_ = nullptr;
void ReinforcementManager::global_init()
{
    instance_ = new ReinforcementManager();
    instance_->init();
}

void ReinforcementManager::init()
{
    this->reinforcements_.emplace(AUDIT_REINFORCEMENT_AUDITD_SWITCH, std::make_shared<AuditdSwitch>());
    this->reinforcements_.emplace(AUDIT_REINFORCEMENT_LOGROTATE_ROTATE, std::make_shared<LogrotateRotate>());
}
}  // namespace Audit
}  // namespace Kiran