/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#include "plugins/cpp/audit/reinforcements/logrotate.h"

namespace KS
{
namespace Audit
{
#define LOGROTATE_CONF_PATH "/etc/logrotate.conf"
#define LOGROTATE_CONF_KEY_ROTATE "rotate"

LogrotateRotate::LogrotateRotate()
{
    this->logrotate_config_ = ConfigPlain::create(LOGROTATE_CONF_PATH);
}

bool LogrotateRotate::get(std::string &args, BRErrorCode &error_code)
{
    if (!this->logrotate_config_)
    {
        error_code = BRErrorCode::ERROR_FAILED;
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
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
}

bool LogrotateRotate::set(const std::string &args, BRErrorCode &error_code)
{
    if (!this->logrotate_config_)
    {
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);
        if (!values[LOGROTATE_CONF_KEY_ROTATE].isInt())
        {
            error_code = BRErrorCode::ERROR_FAILED;
            return false;
        }
        auto value = fmt::format("{0}", values[LOGROTATE_CONF_KEY_ROTATE].asInt());
        this->logrotate_config_->set_value(LOGROTATE_CONF_KEY_ROTATE, value);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

}  // namespace Audit
}  // namespace KS
