/**
 * @file          /kiran-sse-manager/plugins/cpp/config/reinforcements/history.cpp
 * @brief         
 * @author        pengyulong <pengyulong@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#include "plugins/cpp/config/reinforcements/history.h"
#include <json/json.h>
#include <unistd.h>

namespace Kiran
{

#define HISTSIZE_LIMIT_CONF_PATH "/etc/profile"
#define HISTSIZE_LIMIT_CONF_KEY_HISTSIZE "HISTSIZE"


HistsizeLimit::HistsizeLimit()
{
    this->histsize_limit_config_ = ConfigPlain::create(HISTSIZE_LIMIT_CONF_PATH, "=");
}

bool HistsizeLimit::get(const std::string &args,  SSRErrorCode &error_code)
{
    if(!this->histsize_limit_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values;
        auto histsize = this->histsize_limit_config_->get_value(HISTSIZE_LIMIT_CONF_KEY_HISTSIZE);

        value[HISTSIZE_LIMIT_CONF_KEY_HISTSIZE] =  histsize;

        args = StrUtils::json2str(values);

        return true;
    }
    catch(const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
}

bool HistsizeLimit::set(const std::string &args, SSEErrorCode &error_code)
{
    if(!this->histsize_limit_config_)
    {
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }

    try
    {
        Json::Value values = StrUtils::str2json(args);

        if(!values[HISTSIZE_LIMIT_CONF_KEY_HISTSIZE].asInt())
        {
            error_code = SSRErrorCode::ERROR_FAILED;
            return false;
        }

        auto histsize = fmt::format("{0}", values[HISTSIZE_LIMIT_CONF_KEY_HISTSIZE].asInt());
        this->histsize_limit_config_->set_value(HISTSIZE_LIMIT_CONF_KEY_HISTSIZE, histsize);
    }
    catch(const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

}  // namespace Kiran