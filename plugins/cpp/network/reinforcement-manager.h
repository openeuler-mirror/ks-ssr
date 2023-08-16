/**
 * @file          /kiran-ssr-manager/plugins/cpp/network/reinforcement-manager.h
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
namespace Network
{
class ReinforcementManager
{
public:
    ReinforcementManager();
    virtual ~ReinforcementManager(){};

    static ReinforcementManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<SSRReinforcementInterface> get_reinforcement(const std::string& name) { return MapHelper::get_value(this->reinforcements_, name); };

private:
    void init();

private:
    static ReinforcementManager* instance_;

    std::map<std::string, std::shared_ptr<SSRReinforcementInterface>> reinforcements_;
};
}  // namespace Network
}  // namespace Kiran