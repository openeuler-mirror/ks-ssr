/**
 * @file          /kiran-ssr-manager/plugins/cpp/config/reinforcements/cr-password-expired.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "plugin-i.h"

namespace Kiran
{
class CRPasswordExpired : public SSEReinforcementInterface
{
public:
    CRPasswordExpired(){};
    virtual ~CRPasswordExpired(){};

    virtual bool RAMatchRS(const std::string &rs, const std::string &ra) override;

    virtual bool SCMatchRS(const std::string &rs) override;

    virtual bool Reinforce(const std::string &ra, SSEErrorCode &error_code) override;
};
}  // namespace Kiran