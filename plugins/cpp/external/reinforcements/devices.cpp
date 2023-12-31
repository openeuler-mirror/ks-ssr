/**
 * @file          /ks-ssr-manager/plugins/cpp/external/reinforcements/devices.cpp
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/cpp/external/reinforcements/devices.h"
#include <json/json.h>
#include <unistd.h>

namespace KS
{
namespace External
{
#define DEVICE_CONF_PATH "/etc/security/console.perms"
#define DEVICE_CONF_KEY "console"
#define DEVICE_CONF_KEY_CONSOLE "<console>"
#define DEVICE_CONF_KEY_XCONSOLE "<xconsole>"
#define DEVICE_CONF_KEY_CONSOLE_VALUE "tty[0-9][0-9]* vc/[0-9][0-9]* :[0-9]+\\.[0-9]+ :[0-9]+"
#define DEVICE_CONF_KEY_XCONSOLE_VALUE ":[0-9]+\\.[0-9]+ :[0-9]+"

DeviceSwitch::DeviceSwitch()
{
    this->device_config_ = ConfigPlain::create(DEVICE_CONF_PATH, "\\s*=\\s*", "=");
}

bool DeviceSwitch::get(const std::string &args, SSRErrorCode &error_code)
{
    if (!this->device_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        //默认禁用
        auto console = this->device_config_->get_value(DEVICE_CONF_KEY_CONSOLE);
        auto xconsole = this->device_config_->get_value(DEVICE_CONF_KEY_XCONSOLE);

        if (console.empty() && xconsole.empty())
        {
            value[DEVICE_CONF_KEY] = false;
        }
        {
            value[DEVICE_CONF_KEY] = true;
        }
        return true;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
}

bool DeviceSwitch::set(const std::string &args, SSRErrorCode &error_code)
{
    if (!this->device_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);
        RETURN_ERROR_IF_FALSE(values[DEVICE_CONF_KEY].isBool(), SSRErrorCode::ERROR_FAILED);

        if (values[DEVICE_CONF_KEY])
        {
            this->device_config_->set_value(DEVICE_CONF_KEY_CONSOLE, DEVICE_CONF_KEY_CONSOLE_VALUE);
            this->device_config_->set_value(DEVICE_CONF_KEY_XCONSOLE, DEVICE_CONF_KEY_XCONSOLE_VALUE);
        }
        else
        {
            this->device_config_->delete_key(DEVICE_CONF_KEY_CONSOLE);
            this->device_config_->delete_key(DEVICE_CONF_KEY_XCONSOLE);
        }
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

}  // namespace External
}  // namespace KS
