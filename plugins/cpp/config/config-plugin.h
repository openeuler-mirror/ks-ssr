/**
 * @file          /kiran-sse-manager/plugins/cpp/config/config-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/base/base.h"
#include "plugin-i.h"

namespace Kiran
{
namespace Config
{
class SSRPluginConfig : public SSRPluginInterface
{
public:
    SSRPluginConfig(){};
    virtual ~SSRPluginConfig(){};

    virtual void activate() override;

    virtual void deactivate() override;

    virtual std::shared_ptr<SSRReinforcementInterface> get_reinforcement(const std::string &name) { return MapHelper::get_value(this->reinforcements_, name); };
};

private:
    std::map<std::string, std::shared_ptr<SSRReinforcementInterface>> reinforcements_;

}  // namespace Config

}  // namespace Kiran
