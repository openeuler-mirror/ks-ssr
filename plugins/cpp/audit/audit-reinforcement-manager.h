/**
 * @file          /kiran-ssr-manager/plugins/cpp/audit/audit-reinforcement-manager.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "reinforcement-i.h"

// audit reinforcements manager

namespace Kiran
{
class AuditReinforcementManager
{
public:
    AuditReinforcementManager();
    virtual ~AuditReinforcementManager();

    static AuditReinforcementManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<SSRReinforcementInterface> get_reinforcement(const std::string& name) { return MapHelper::get_value(this->reinforcements_, name); };

private:
    void init();

private:
    static AuditReinforcementManager* instance_;

    std::map<std::string, std::shared_ptr<SSRReinforcementInterface>> reinforcements_;
};
}  // namespace Kiran