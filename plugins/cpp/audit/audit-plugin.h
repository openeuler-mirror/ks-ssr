/**
 * @file          /kiran-ssr-manager/plugins/cpp/audit/audit-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#include "plugin-i.h"

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

    virtual std::string execute(const std::string &in_json) override;
};

}  // namespace Audit

}  // namespace Kiran
