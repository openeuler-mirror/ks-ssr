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

#include "plugins/cpp/config/reinforcements/history.h"
#include <json/json.h>
#include <unistd.h>

namespace KS
{
#define HISTORY_SIZE_LIMIT_CONF_PATH "/etc/profile"
#define HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE "HISTSIZE"

HistorySizeLimit::HistorySizeLimit()
{
    this->history_size_limit_config_ = ConfigPlain::create(HISTORY_SIZE_LIMIT_CONF_PATH, "=");
}

bool HistorySizeLimit::get(const std::string &args, BRErrorCode &error_code)
{
    if (!this->history_size_limit_config_)
    {
        RETURN_ERROR_IF_FALSE(false, BRErrorCode::ERROR_FAILED);
    }

    try
    {
        Json::Value values;
        auto histsize = this->history_size_limit_config_->get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE);
        value[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE] = histsize;
        args = StrUtils::json2str(values);
        return true;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        RETURN_ERROR_IF_FALSE(false, BRErrorCode::ERROR_FAILED);
    }
}

bool HistorySizeLimit::set(const std::string &args, BRErrorCode &error_code)
{
    if (!this->history_size_limit_config_)
    {
        RETURN_ERROR_IF_FALSE(false, BRErrorCode::ERROR_FAILED);
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);

        RETURN_ERROR_IF_FALSE(values[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE].isInt(), BRErrorCode::ERROR_FAILED);

        auto histsize = fmt::format("{0}", values[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE].asInt());
        this->history_size_limit_config_->set_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE, histsize);

        return true;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        RETURN_ERROR_IF_FALSE(false, BRErrorCode::ERROR_FAILED);
    }
}

}  // namespace KS