/**
 * @file          /ks-ssr-manager/plugins/cpp/network/network-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "plugins/cpp/network/network-plugin.h"
#include <json/json.h>
#include "lib/base/base.h"
#include "plugins/cpp/network/reinforcements/firewalld.h"
#include "plugins/cpp/network/reinforcements/sysctl.h"

PLUGIN_EXPORT_FUNC_DEF(KS::Network::SSRPluginNetwork);

namespace KS
{
namespace Network
{
#define NETWORK_REINFORCEMENT_FIREWALLD_SWITCH "network-firewalld-switch"
#define NETWORK_REINFORCEMENT_FIREWALLD_ICMP_TIMESTAMP "network-firewalld-icmp-timestamp"
#define NETWORK_REINFORCEMENT_SYSCTL_REDIRECT "network-sysctl-redirect"
#define NETWORK_REINFORCEMENT_SYSCTL_SOURCE_ROUTE "network-sysctl-source-route"

void SSRPluginNetwork::activate()
{
    this->reinforcements_ = std::map<std::string, std::shared_ptr<SSRReinforcementInterface>>(
        {{NETWORK_REINFORCEMENT_FIREWALLD_SWITCH, std::make_shared<FirewalldSwitch>()},
         {NETWORK_REINFORCEMENT_FIREWALLD_ICMP_TIMESTAMP, std::make_shared<FirewalldICMPTimestamp>()},
         {NETWORK_REINFORCEMENT_SYSCTL_REDIRECT, std::make_shared<SysctlRedirect>()},
         {NETWORK_REINFORCEMENT_SYSCTL_SOURCE_ROUTE, std::make_shared<SysctlSourceRoute>()}});
}

void SSRPluginNetwork::deactivate()
{
    this->reinforcements_.clear();
}
}  // namespace Network
}  // namespace KS
