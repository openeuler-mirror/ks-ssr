/**
 * @file          /ks-br-manager/plugins/cpp/external/reinforcements/devices.h
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
#include "lib/config/config-plain.h"

namespace KS
{
namespace External
{
class DeviceSwitch : public BRReinforcementInterface
{
public:
    DeviceSwitch(){};
    virtual ~DeviceSwitch(){};

    virtual bool get(std::string &args, BRErrorCode &error_code);

    virtual bool set(const std::string &args, BRErrorCode &error_code);

private:
    std::shared_ptr<ConfigPlain> device_config_;
};

}  // namespace External
}  // namespace KS
