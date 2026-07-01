/**
 * @file          /kiran-sse-manager/plugins/cpp/audit/reinforcements/audit-logrotate.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "plugins/cpp/audit/reinforcements/audit-logrotate.h"

namespace Kiran
{
#define LOGROTATE_CONF_PATH "/etc/logrotate.conf"
#define LOGROTATE_CONF_KEY_ROTATE "rotate"

AuditLogrotateRotate::AuditLogrotateRotate()
{
    this->logrotate_config_ = ConfigPlain::create(LOGROTATE_CONF_PATH);
}

bool AuditLogrotateRotate::get(std::string &args, SSEErrorCode &error_code)
{
    if (!this->logrotate_config_)
    {
        error_code = SSEErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        auto rotate = this->logrotate_config_->get_integer(LOGROTATE_CONF_KEY_ROTATE);
        values[LOGROTATE_CONF_KEY_ROTATE] = rotate;
        args = StrUtils::json2str(values);
        return true;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSEErrorCode::ERROR_FAILED;
        return false;
    }
}

bool AuditLogrotateRotate::set(const std::string &args, SSEErrorCode &error_code)
{
    if (!this->logrotate_config_)
    {
        error_code = SSEErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);
        if (!values[LOGROTATE_CONF_KEY_ROTATE].isInt())
        {
            error_code = SSEErrorCode::ERROR_FAILED;
            return false;
        }
        auto value = fmt::format("{0}", values[LOGROTATE_CONF_KEY_ROTATE].asInt());
        this->logrotate_config_->set_value(LOGROTATE_CONF_KEY_ROTATE, value);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSEErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

}  // namespace Kiran
