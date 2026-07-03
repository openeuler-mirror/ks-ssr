/**
 * @file          /ks-br-manager/plugins/cpp/external/external-plugin.h
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "br-plugin-i.h"
#include "lib/base/base.h"

namespace KS
{
namespace External
{
class BRPluginExternal : public BRPluginInterface
{
public:
    BRPluginExternal(){};
    virtual ~BRPluginExternal(){};

    virtual void activate();

    virtual void deactivate();

    virtual std::shared_ptr<BRReinforcementInterface> get_reinforcement(const std::string &name) { return MapHelper::get_value(this->reinforcements_, name); };

private:
    std::map<std::string, std::shared_ptr<BRReinforcementInterface>> reinforcements_;
};

}  // namespace External
}  // namespace KS
