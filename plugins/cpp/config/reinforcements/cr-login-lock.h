/**
 * @file          /kiran-sse-manager/plugins/cpp/config/reinforcements/cr-login-lock.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "reinforcement-i.h"

namespace Kiran
{
class CRLoginLock : public SSEReinforcementInterface
{
public:
    CRLoginLock(){};
    virtual ~CRLoginLock(){};

    virtual bool RAMatchRS(const std::string &rs, const std::string &ra) override;

    virtual bool SCMatchRS(const std::string &rs) override;

    virtual bool Reinforce(const std::string &ra, SSEErrorCode &error_code) override;
};
}  // namespace Kiran