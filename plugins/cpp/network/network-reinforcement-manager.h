/**
 * @file          /kiran-sse-manager/plugins/cpp/network/network-reinforcement-manager.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */
#pragma once

#include "lib/base/base.h"
#include "reinforcement-i.h"

// audit reinforcements manager

namespace Kiran
{
class NetworkReinforcementManager
{
public:
    NetworkReinforcementManager();
    virtual ~NetworkReinforcementManager(){};

    static NetworkReinforcementManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<SSEReinforcementInterface> get_reinforcement(const std::string& name) { return MapHelper::get_value(this->reinforcements_, name); };

private:
    void init();

private:
    static NetworkReinforcementManager* instance_;

    std::map<std::string, std::shared_ptr<SSEReinforcementInterface>> reinforcements_;
};
}  // namespace Kiran