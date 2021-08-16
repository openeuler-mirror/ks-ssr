/**
 * @file          /kiran-ssr-manager/plugins/cpp/audit/reinforcements/audit-autitd.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include <json/json.h>
#include <unistd.h>
#include "plugins/cpp/audit/reinforcements/audit-auditd.h"

namespace Kiran
{
#define AUDITD_UNIT_NAME "auditd.service"
#define AUDITD_JSON_KEY_ENABLED "enabled"
#define AUDITD_JSON_KEY_ACTIVE "acitve"

AuditAuditdSwitch::AuditAuditdSwitch()
{
    this->systemd_proxy_ = DBusSystemdProxy::get_default();
}

bool AuditAuditdSwitch::get(std::string &args, SSRErrorCode &error_code)
{
    Json::Value values;

    try
    {
        // 开机自动启动
        auto unit_file_state = this->systemd_proxy_->get_unit_file_state(AUDITD_UNIT_NAME);
        values[AUDITD_JSON_KEY_ENABLED] = (unit_file_state == "enabled");

        // 是否运行
        auto state_str = this->systemd_proxy_->get_unit_active_state(AUDITD_UNIT_NAME);
        values[AUDITD_JSON_KEY_ACTIVE] = (state_str == "active" || state_str == "activating");

        args = StrUtils::json2str(values);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        error_code = SSRErrorCode::ERROR_PLUGIN_AUDIT_GET_JSON_ERROR;
        return false;
    }
    return true;
}

bool AuditAuditdSwitch::set(const std::string &args, SSRErrorCode &error_code)
{
    try
    {
        Json::Value values = StrUtils::str2json(args);
        if (values[AUDITD_JSON_KEY_ENABLED].isBool())
        {
            auto enabled = values[AUDITD_JSON_KEY_ENABLED].asBool();
            this->systemd_proxy_->enable_unit_file(AUDITD_UNIT_NAME, enabled);
        }

        if (values[AUDITD_JSON_KEY_ACTIVE].isBool())
        {
            auto active = values[AUDITD_JSON_KEY_ACTIVE].asBool();
            if (active)
            {
                return this->systemd_proxy_->start_unit(AUDITD_UNIT_NAME);
            }
            else
            {
                return this->systemd_proxy_->stop_unit(AUDITD_UNIT_NAME);
            }
        }
        return true;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        error_code = SSRErrorCode::ERROR_PLUGIN_AUDIT_SET_JSON_ERROR;
        return false;
    }
}

}  // namespace Kiran