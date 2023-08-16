/**
 * @file          /kiran-ssr-manager/plugins/cpp/network/reinforcement-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "plugins/cpp/network/reinforcement-manager.h"
#include "plugins/cpp/network/reinforcements/firewalld.h"
#include "plugins/cpp/network/reinforcements/sysctl.h"

namespace Kiran
{
namespace Network
{
#define NETWORK_REINFORCEMENT_FIREWALLD_SWITCH "network-firewalld-switch"
#define NETWORK_REINFORCEMENT_FIREWALLD_ICMP_TIMESTAMP "network-firewalld-icmp-timestamp"
#define NETWORK_REINFORCEMENT_SYSCTL_REDIRECT "network-sysctl-redirect"
#define NETWORK_REINFORCEMENT_SYSCTL_SOURCE_ROUTE "network-sysctl-source-route"

ReinforcementManager::ReinforcementManager()
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
    this->reinforcements_.emplace(NETWORK_REINFORCEMENT_FIREWALLD_SWITCH, std::make_shared<FirewalldSwitch>());
    this->reinforcements_.emplace(NETWORK_REINFORCEMENT_FIREWALLD_ICMP_TIMESTAMP, std::make_shared<FirewalldICMPTimestamp>());
    this->reinforcements_.emplace(NETWORK_REINFORCEMENT_SYSCTL_REDIRECT, std::make_shared<SysctlRedirect>());
    this->reinforcements_.emplace(NETWORK_REINFORCEMENT_SYSCTL_SOURCE_ROUTE, std::make_shared<SysctlSourceRoute>());
}

}  // namespace Network

}  // namespace Kiran
