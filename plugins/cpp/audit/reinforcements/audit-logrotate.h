/**
 * @file          /kiran-sse-manager/plugins/cpp/audit/reinforcements/audit-logrotate.h
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
class AuditLogrotateRotate : public SSEReinforcementInterface
{
public:
    AuditLogrotateRotate();
    virtual ~AuditLogrotateRotate(){};

    virtual bool get(std::string &args, SSEErrorCode &error_code);
    virtual bool set(const std::string &args, SSEErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> logrotate_config_;
};

}  // namespace  Kiran
