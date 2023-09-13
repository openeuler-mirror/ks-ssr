/**
 * @file          /ks-br-manager/plugins/cpp/network/network-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "br-plugin-i.h"
#include "lib/base/base.h"

namespace KS
{
namespace Network
{
class BRPluginNetwork : public BRPluginInterface
{
public:
    BRPluginNetwork(){};
    virtual ~BRPluginNetwork(){};

    virtual void activate();

    virtual void deactivate();

    virtual std::shared_ptr<BRReinforcementInterface> get_reinforcement(const std::string &name) { return MapHelper::get_value(this->reinforcements_, name); };

private:
    std::map<std::string, std::shared_ptr<BRReinforcementInterface>> reinforcements_;
};
}  // namespace Network

}  // namespace KS
