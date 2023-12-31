/**
 * @file          /ks-ssr-manager/plugins/cpp/config/config-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/base/base.h"
#include "ssr-plugin-i.h"

namespace KS
{
namespace Config
{
class SSRPluginConfig : public SSRPluginInterface
{
public:
    SSRPluginConfig(){};
    virtual ~SSRPluginConfig(){};

    virtual void activate();

    virtual void deactivate();

    virtual std::shared_ptr<SSRReinforcementInterface> get_reinforcement(const std::string &name) { return MapHelper::get_value(this->reinforcements_, name); };
};

private:
std::map<std::string, std::shared_ptr<SSRReinforcementInterface>> reinforcements_;

}  // namespace Config

}  // namespace KS
