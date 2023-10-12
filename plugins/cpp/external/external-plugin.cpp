/**
 * @file          /ks-br-manager/plugins/cpp/config/external-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/cpp/external/external-plugin.h"
#include <json/json.h>
#include "plugins/cpp/external/reinforcements/devices.cpp"
#include "plugins/cpp/external/reinforcements/login-restrictions.cpp"

PLUGIN_EXPORT_FUNC_DEF(KS::External::BRPluginExternal);

namespace KS
{
namespace External
{
#define EXTERNAL_REINFORCEMENT_LOGIN_RESTRICTIONS "external-login-restrictions"
#define EXTERNAL_REINFORCEMENT_DEVICES_SWITCH "external-devices-switch"
#define EXTERNAL_REINFORCEMENT_LOGIN_TIMEOUT "external-login-timeout"

void BRPluginExternal::activate()
{
    this->reinforcements_ = std::map<std::string, std::shared_ptr<BRReinforcementInterface>>(
        {{EXTERNAL_REINFORCEMENT_LOGIN_RESTRICTIONS, std::make_shared<LoginRestrictions>()},
         {EXTERNAL_REINFORCEMENT_LOGIN_TIMEOUT, std::make_shared<LoginTimeout>()},
         {EXTERNAL_REINFORCEMENT_DEVICES_SWITCH, std::make_shared<DeviceSwitch>()}});
}

void BRPluginExternal::deactivate()
{
    this->reinforcements_.clear();
}

}  // namespace External
}  // namespace KS
