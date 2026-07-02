/**
 * @file          /ks-ssr-manager/plugins/cpp/network/network-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "lib/base/base.h"
#include "ssr-plugin-i.h"

namespace KS
{
namespace Network
{
class SSRPluginNetwork : public SSRPluginInterface
{
public:
    SSRPluginNetwork(){};
    virtual ~SSRPluginNetwork(){};

    virtual void activate();

    virtual void deactivate();

    virtual std::shared_ptr<SSRReinforcementInterface> get_reinforcement(const std::string &name) { return MapHelper::get_value(this->reinforcements_, name); };

private:
    std::map<std::string, std::shared_ptr<SSRReinforcementInterface>> reinforcements_;
};
}  // namespace Network

}  // namespace KS
