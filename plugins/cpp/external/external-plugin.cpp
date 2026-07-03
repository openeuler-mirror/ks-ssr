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
