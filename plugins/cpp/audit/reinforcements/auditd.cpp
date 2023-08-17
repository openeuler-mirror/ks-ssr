/**
 * @file          /kiran-ssr-manager/plugins/cpp/audit/reinforcements/auditd.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "plugins/cpp/audit/reinforcements/auditd.h"
#include <json/json.h>
#include <unistd.h>

namespace Kiran
{
namespace Audit
{
#define AUDITD_UNIT_NAME "auditd.service"
#define AUDITD_JSON_KEY_ENABLED "enabled"

AuditdSwitch::AuditdSwitch()
{
    this->systemd_proxy_ = std::make_shared<DBusSystemdProxy>();
}

bool AuditdSwitch::get(std::string &args, SSRErrorCode &error_code)
{
    Json::Value values;

    try
    {
        // 开机自动启动
        // auto unit_file_state = this->systemd_proxy_->get_unit_file_state(AUDITD_UNIT_NAME);
        // values[AUDITD_JSON_KEY_ENABLED] = (unit_file_state == "enabled");

        // 是否运行
        auto state_str = this->systemd_proxy_->get_unit_active_state(AUDITD_UNIT_NAME);
        values[AUDITD_JSON_KEY_ENABLED] = (state_str == "active" || state_str == "activating");

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

bool AuditdSwitch::set(const std::string &args, SSRErrorCode &error_code)
{
    try
    {
        Json::Value values = StrUtils::str2json(args);
        if (values[AUDITD_JSON_KEY_ENABLED].isBool())
        {
            auto enabled = values[AUDITD_JSON_KEY_ENABLED].asBool();
            this->systemd_proxy_->enable_unit_file(AUDITD_UNIT_NAME, enabled);

            if (enabled)
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

}  // namespace Audit
}  // namespace Kiran