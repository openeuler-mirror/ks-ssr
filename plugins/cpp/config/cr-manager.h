/**
 * @file          /kiran-sse-manager/plugins/cpp/config/cr-manager.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "reinforcement-i.h"

// config reinforcement manager

namespace Kiran
{
class CRManager
{
public:
    CRManager();
    virtual ~CRManager();

    static CRManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<SSEReinforcementInterface> get_reinforcement(const std::string& name) { return MapHelper::get_value(this->reinforcements_, name); };

private:
    void init();

private:
    static CRManager* instance_;

    std::map<std::string, std::shared_ptr<SSEReinforcementInterface>> reinforcements_;
};
}  // namespace Kiran