/**
 * @file          /kiran-sse-manager/plugins/cpp/network/reinforcements/network-firewall.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "plugins/cpp/network/reinforcements/network-firewall.h"

namespace Kiran
{
#define FIREWALLD_UNIT_NAME "firewalld.service"
#define FIREWALLD_JSON_KEY_ENABLED "enabled"
#define FIREWALL_JSON_KEY_ACTIVE "active"

NetworkFirewallSwitch::NetworkFirewallSwitch()
{
    this->systemd_proxy_ = DBusSystemdProxy::get_default();
}

bool NetworkFirewallSwitch::get(std::string &args, SSEErrorCode &error_code)
{
    Json::Value values;

    try
    {
        // 开机自动启动
        auto unit_file_state = this->systemd_proxy_->get_unit_file_state(FIREWALLD_UNIT_NAME);
        values[FIREWALLD_JSON_KEY_ENABLED] = (unit_file_state == "enabled");

        // 是否运行
        auto state_str = this->systemd_proxy_->get_unit_active_state(FIREWALLD_UNIT_NAME);
        values[FIREWALL_JSON_KEY_ACTIVE] = (state_str == "active" || state_str == "activating");

        args = StrUtils::json2str(values);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        error_code = SSEErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

bool NetworkFirewallSwitch::set(const std::string &args, SSEErrorCode &error_code)
{
    try
    {
        Json::Value values = StrUtils::str2json(args);
        if (values[FIREWALLD_JSON_KEY_ENABLED].isBool())
        {
            auto enabled = values[FIREWALLD_JSON_KEY_ENABLED].asBool();
            this->systemd_proxy_->enable_unit_file(FIREWALLD_UNIT_NAME, enabled);
        }

        if (values[FIREWALL_JSON_KEY_ACTIVE].isBool())
        {
            auto active = values[FIREWALL_JSON_KEY_ACTIVE].asBool();
            if (active)
            {
                return this->systemd_proxy_->start_unit(FIREWALLD_UNIT_NAME);
            }
            else
            {
                return this->systemd_proxy_->stop_unit(FIREWALLD_UNIT_NAME);
            }
        }
        return true;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        error_code = SSEErrorCode::ERROR_FAILED;
        return false;
    }
}
}  // namespace Kiran
