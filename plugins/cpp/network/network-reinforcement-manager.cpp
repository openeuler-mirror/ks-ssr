/**
 * @file          /kiran-ssr-manager/plugins/cpp/network/network-reinforcement-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "plugins/cpp/network/network-reinforcement-manager.h"

namespace Kiran
{
// #define AUDIT_REINFORCEMENT_AUDITD_SWITCH "audit-auditd-switch"

NetworkReinforcementManager::NetworkReinforcementManager()
{
}

NetworkReinforcementManager* NetworkReinforcementManager::instance_ = nullptr;
void NetworkReinforcementManager::global_init()
{
    instance_ = new NetworkReinforcementManager();
    instance_->init();
}

void NetworkReinforcementManager::init()
{
    // this->reinforcements_.emplace(AUDIT_REINFORCEMENT_AUDITD_SWITCH, std::make_shared<AuditAuditdSwitch>());
}
}  // namespace Kiran