/**
 * @file          /kiran-sse-manager/plugins/cpp/config/cr-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#include "plugins/cpp/config/cr-manager.h"
#include "plugins/cpp/config/reinforcements/cr-login-lock.h"
#include "plugins/cpp/config/reinforcements/cr-password-complexity.h"
#include "plugins/cpp/config/reinforcements/cr-password-expired.h"

namespace Kiran
{
#define CR_LOGIN_LOCK_GROUP_NAME "config-login-lock"
#define CR_PASSWORD_COMPLEX_GROUP_NAME "config-password-complexity"
#define CR_PASSWORD_EXPIRED "config-password-expired"

CRManager::CRManager()
{
}

CRManager::~CRManager()
{
}

CRManager* CRManager::instance_ = nullptr;
void CRManager::global_init()
{
    instance_ = new CRManager();
    instance_->init();
}

void CRManager::init()
{
    this->reinforcements_.emplace(CR_LOGIN_LOCK_GROUP_NAME, std::make_shared<CRLoginLock>());
    this->reinforcements_.emplace(CR_PASSWORD_COMPLEX_GROUP_NAME, std::make_shared<CRPasswordComplextiy>());
    this->reinforcements_.emplace(CR_PASSWORD_EXPIRED, std::make_shared<CRPasswordExpired>());
}
}  // namespace Kiran