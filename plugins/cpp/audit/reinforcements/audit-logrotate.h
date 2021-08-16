/**
 * @file          /kiran-ssr-manager/plugins/cpp/audit/reinforcements/audit-logrotate.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "lib/config/config-plain.h"
#include "reinforcement-i.h"

namespace Kiran
{
class AuditLogrotateRotate : public SSRReinforcementInterface
{
public:
    AuditLogrotateRotate();
    virtual ~AuditLogrotateRotate(){};

    virtual bool get(std::string &args, SSRErrorCode &error_code);
    virtual bool set(const std::string &args, SSRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> logrotate_config_;
};

}  // namespace  Kiran
