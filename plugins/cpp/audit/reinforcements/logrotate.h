/**
 * @file          /ks-br-manager/plugins/cpp/audit/reinforcements/logrotate.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "lib/config/config-plain.h"

namespace KS
{
namespace Audit
{
class LogrotateRotate : public BRReinforcementInterface
{
public:
    LogrotateRotate();
    virtual ~LogrotateRotate(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);
    virtual bool set(const std::string &args, BRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> logrotate_config_;
};

}  // namespace Audit
}  // namespace KS
