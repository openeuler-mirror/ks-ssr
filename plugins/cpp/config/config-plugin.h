/**
 * @file          /ks-br-manager/plugins/cpp/config/config-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "br-plugin-i.h"
#include "lib/base/base.h"

namespace KS
{
namespace Config
{
class BRPluginConfig : public BRPluginInterface
{
public:
    BRPluginConfig(){};
    virtual ~BRPluginConfig(){};

    virtual void activate();

    virtual void deactivate();

    virtual std::shared_ptr<BRReinforcementInterface> get_reinforcement(const std::string &name) { return MapHelper::get_value(this->reinforcements_, name); };
};

private:
std::map<std::string, std::shared_ptr<BRReinforcementInterface>> reinforcements_;

}  // namespace Config

}  // namespace KS
