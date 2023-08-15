/**
 * @file          /kiran-sse-manager/plugins/cpp/config/config-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugin-i.h"

namespace Kiran
{
class SSEPluginConfig : public SSEPluginInterface
{
public:
    SSEPluginConfig(){};
    virtual ~SSEPluginConfig(){};

    virtual void activate() override;

    virtual void deactivate() override;

    virtual std::string execute(const std::string &in_json) override;
};

}  // namespace Kiran
