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

#include "plugins/cpp/network/reinforcements/sysctl.h"

namespace KS
{
namespace Network
{
#define SYSCTL_COMMAND "/usr/sbin/sysctl"
#define SYSCTL_JSON_KEY_ENABLED "enabled"

#define SYSCTL_CONFI_FILE "/etc/sysctl.d/10-sysctl-br.conf"
#define SYSCTL_CONFIG_FIELD_PARTTERN "\\s*=\\s*"
#define SYSCTL_ACCEPT_REDIRECTS_PATTERN "net.ipv4.conf.*.accept_redirects"
#define SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN "net.ipv4.conf.*.accept_source_route"

Sysctl::Sysctl()
{
    this->sysctl_config_ = ConfigPlain::create(SYSCTL_CONFI_FILE, SYSCTL_CONFIG_FIELD_PARTTERN, " = ");
}

std::vector<SysctlRedirect::SysctlVar> Sysctl::get_vars_by_pattern(const std::string &pattern)
{
    std::vector<SysctlVar> vars;
    std::string standard_output;
    std::string standard_error;
    std::vector<std::string> argv = {SYSCTL_COMMAND, "-a", "-r", pattern};

    RETURN_VAL_IF_FALSE(MiscUtils::spawn_sync(argv, &standard_output, &standard_error), vars);
    KLOG_DEBUG("standard output: %s. error: %s.", standard_output.c_str(), standard_error.c_str());
    auto lines = StrUtils::split_lines(standard_output);
    auto regex = Glib::Regex::create(SYSCTL_CONFIG_FIELD_PARTTERN);
    for (const auto &line : lines)
    {
        std::vector<std::string> fields = regex->split(line);
        CONTINUE_IF_TRUE(fields.size() != 2);
        vars.push_back(std::make_pair(fields[0], fields[1]));
    }
    return vars;
}

SysctlRedirect::SysctlRedirect()
{
}

bool SysctlRedirect::get(std::string &args, BRErrorCode &error_code)
{
    auto redirect_vars = this->get_vars_by_pattern(SYSCTL_ACCEPT_REDIRECTS_PATTERN);
    RETURN_ERROR_IF_TRUE(redirect_vars.empty(), BRErrorCode::ERROR_FAILED);

    try
    {
        bool enabled = false;
        Json::Value values;
        for (const auto &var : redirect_vars)
        {
            if (var.second == "1")
            {
                enabled = true;
                break;
            }
        }
        values[SYSCTL_JSON_KEY_ENABLED] = enabled;
        args = StrUtils::json2str(values);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

bool SysctlRedirect::set(const std::string &args, BRErrorCode &error_code)
{
    auto redirect_vars = this->get_vars_by_pattern(SYSCTL_ACCEPT_REDIRECTS_PATTERN);
    RETURN_ERROR_IF_TRUE(redirect_vars.empty(), BRErrorCode::ERROR_FAILED);

    try
    {
        auto values = StrUtils::str2json(args);
        RETURN_ERROR_IF_TRUE(!values[SYSCTL_JSON_KEY_ENABLED].isBool(), BRErrorCode::ERROR_FAILED);
        auto enabled = values[SYSCTL_JSON_KEY_ENABLED].asBool();

        // 写入文件中
        for (const auto &var : redirect_vars)
        {
            RETURN_ERROR_IF_FALSE(this->sysctl_config_->set_value(var.first, enabled ? "1" : "0"), BRErrorCode::ERROR_FAILED);
        }

        // 从文件中刷新
        std::vector<std::string> argv = {SYSCTL_COMMAND, "--load", SYSCTL_CONFI_FILE};
        RETURN_ERROR_IF_TRUE(!MiscUtils::spawn_sync(argv), BRErrorCode::ERROR_FAILED);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

SysctlSourceRoute::SysctlSourceRoute()
{
}

bool SysctlSourceRoute::get(std::string &args, BRErrorCode &error_code)
{
    auto redirect_vars = this->get_vars_by_pattern(SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN);
    RETURN_ERROR_IF_TRUE(redirect_vars.empty(), BRErrorCode::ERROR_FAILED);

    try
    {
        bool enabled = false;
        Json::Value values;
        for (const auto &var : redirect_vars)
        {
            if (var.second == "1")
            {
                enabled = true;
                break;
            }
        }
        values[SYSCTL_JSON_KEY_ENABLED] = enabled;
        args = StrUtils::json2str(values);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

bool SysctlSourceRoute::set(const std::string &args, BRErrorCode &error_code)
{
    auto redirect_vars = this->get_vars_by_pattern(SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN);
    RETURN_ERROR_IF_TRUE(redirect_vars.empty(), BRErrorCode::ERROR_FAILED);

    try
    {
        auto values = StrUtils::str2json(args);
        RETURN_ERROR_IF_TRUE(!values[SYSCTL_JSON_KEY_ENABLED].isBool(), BRErrorCode::ERROR_FAILED);
        auto enabled = values[SYSCTL_JSON_KEY_ENABLED].asBool();

        // 写入文件中
        for (const auto &var : redirect_vars)
        {
            RETURN_ERROR_IF_FALSE(this->sysctl_config_->set_value(var.first, enabled ? "1" : "0"), BRErrorCode::ERROR_FAILED);
        }

        // 从文件中刷新
        std::vector<std::string> argv = {SYSCTL_COMMAND, "--load", SYSCTL_CONFI_FILE};
        RETURN_ERROR_IF_TRUE(!MiscUtils::spawn_sync(argv), BRErrorCode::ERROR_FAILED);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

}  // namespace Network
}  // namespace KS
