/**
 * @file          /ks-ssr-manager/plugins/cpp/external/external-plugin.h
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/base/base.h"
#include "ssr-plugin-i.h"

namespace KS
{
namespace External
{
class SSRPluginExternal : public SSRPluginInterface
{
public:
    SSRPluginExternal(){};
    virtual ~SSRPluginExternal(){};

    virtual void activate();

    virtual void deactivate();

    virtual std::shared_ptr<SSRReinforcementInterface> get_reinforcement(const std::string &name) { return MapHelper::get_value(this->reinforcements_, name); };

private:
    std::map<std::string, std::shared_ptr<SSRReinforcementInterface>> reinforcements_;
};

}  // namespace External
}  // namespace KS
