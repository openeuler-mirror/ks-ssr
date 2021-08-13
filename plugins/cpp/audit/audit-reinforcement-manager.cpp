/**
 * @file          /kiran-sse-manager/plugins/cpp/config/cr-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#include "plugins/cpp/audit/audit-reinforcement-manager.h"

namespace Kiran
{
#define CR_LOGIN_LOCK_GROUP_NAME "config-login-lock"

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
    // this->reinforcements_.emplace(CR_LOGIN_LOCK_GROUP_NAME, std::make_shared<CRLoginLock>());
}
}  // namespace Kiran