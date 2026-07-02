/**
 * @file          /ks-ssr-manager/plugins/cpp/config/reinforcements/history.h
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "lib/base/base.h"
#include "lib/config/config-plain.h"
#include "reinforcement-i.h"

namespace KS
{
namespace Config
{
class HistorySizeLimit : public SSRReinforcementInterface
{
public:
    HistorySizeLimit(){};
    virtual ~HistorySizeLimit(){};

    virtual bool get(std::string &args, SSRErrorCode &error_code);

    virtual bool set(const std::string &args, SSRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> history_size_limit_config_;
};

}  // namespace Config
}  // namespace KS