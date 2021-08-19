/**
 * @file          /kiran-sse-manager/plugins/cpp/config/reinforcements/history.h
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "lib/base/base.h"
#include "lib/config/config-plain.h"
#include "reinforcement-i.h"

namespace Kiran
{
namespace Config
{
class HistsizeLimit : public SSEReinforcementInterface
{
public:
    HistsizeLimit(){};
    virtual ~HistsizeLimit(){};

    virtual bool get(std::string &args, SSRErrorCode &error_code);

    virtual bool set(const std::string &args, SSRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> histsize_limit_config_;
};

}  // namespace Config
}  // namespace Kiran