/**
 * @file          /kiran-ssr-manager/plugins/cpp/audit/audit-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/base/base.h"

namespace Kiran
{
namespace Audit
{
class SSRPluginAudit : public SSRPluginInterface
{
public:
    SSRPluginAudit(){};
    virtual ~SSRPluginAudit(){};

    virtual void activate() override;

    virtual void deactivate() override;

    virtual std::shared_ptr<SSRReinforcementInterface> get_reinforcement(const std::string &name) { return MapHelper::get_value(this->reinforcements_, name); };

private:
    std::map<std::string, std::shared_ptr<SSRReinforcementInterface>> reinforcements_;
};

}  // namespace Audit

}  // namespace Kiran
