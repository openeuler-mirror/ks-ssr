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

#include "plugins/cpp/config/reinforcements/login-lock.h"
#include <json/json.h>
#include <unistd.h>

namespace KS
{
namespace Config
{
#define LOGIN_LOCK_CONF_PATH "/etc/pam.d/system-auth"
#define LOGIN_LOCK_CONF_KEY_TALLY "auth        required                                     pam_tally.so"

LoginLock::LoginLock()
{
    this->login_lock_config_ = ConfigPAM::create(LOGIN_LOCK_CONF_KEY_TALLY);
}

bool LoginLock::get(const std::string &args, BRErrorCode &error_code)
{
    if (!this->password_complextiy_config_)
    {
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        auto tally = this->login_lock_config_->get_all_value(LOGIN_LOCK_CONF_KEY_TALLY);
        values[LOGIN_LOCK_CONF_KEY_TALLY] = tally;
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

// XXX：有关操作数的问题，需要对有操作数的内容进行处理。
bool LoginLock::set(const std::string &args, BRErrorCode &error_code)
{
    if (!this->password_complextiy_config_)
    {
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);
        auto keys = values.getMemberNames();

        for (auto iter : keys)
        {
            this->login_lock_config_->set_value(iter, values[iter]);
        }
        return true;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
}

}  // namespace Config
}  // namespace KS