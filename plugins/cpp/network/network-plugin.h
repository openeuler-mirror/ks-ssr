/**
 * @file          /kiran-sse-manager/plugins/cpp/network/network-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "plugin-i.h"

namespace Kiran
{
class SSEPluginNetwork : public SSEPluginInterface
{
public:
    SSEPluginNetwork(){};
    virtual ~SSEPluginNetwork(){};

    virtual void activate() override;

    virtual void deactivate() override;

    virtual std::string execute(const std::string &in_json) override;
};

}  // namespace Kiran
