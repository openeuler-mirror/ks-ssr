/**
 * @file          /kiran-sse-manager/plugins/cpp/config/config-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/cpp/config/config-plugin.h"
#include <json/json.h>
#include "lib/base/base.h"
#include "plugins/cpp/config/reinforcements/password.h"
#include "plugins/cpp/config/reinforcements/login-lock.h"
#include "plugins/cpp/config/reinforcements/history.h"

PLUGIN_EXPORT_FUNC_DEF(Kiran::Config::SSRPluginConfig);

namespace Kiran
{
namespace Config
{

#define CONFIG_REINFORCEMENT_LOGIN_LOCK "config-login-lock"
#define CONFIG_REINFORCEMENT_PASSWORD_COMPLEX "config-password-complexity"
#define CONFIG_REINFORCEMENT_PASSWORD_EXPIRED "config-password-expired"
#define CONFIG_REINFORCEMENT_HISTORY_SIZE "config-history-size"

void SSRPluginConfig::activate()
{
    this->reinforcements_ = std::map<std::string, std::shared_ptr<SSRReinforcementInterface>>(
        {{CONFIG_REINFORCEMENT_LOGIN_LOCK, std::make_shared<LoginLock>()},
         {CONFIG_REINFORCEMENT_PASSWORD_COMPLEX, std::make_shared<PasswordComplextiy>()},
         {CONFIG_REINFORCEMENT_PASSWORD_EXPIRED, std::make_shared<PasswordExpired>()},
         {CONFIG_REINFORCEMENT_HISTORY_SIZE, std::make_shared<HistsizeLimit>()}});
}

void SSRPluginNetwork::deactivate()
{
    this->reinforcements_.clear();
}

}  // namespace Config

}  // namespace Kiran
