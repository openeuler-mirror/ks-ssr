/**
 * @file          /kiran-ssr-manager/plugins/cpp/network/reinforcements/sysctl.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "plugins/cpp/network/reinforcements/sysctl.h"

namespace Kiran
{
namespace Network
{
#define SYSCTL_COMMAND "/usr/sbin/sysctl"
#define SYSCTL_JSON_KEY_ENABLED "enabled"

#define SYSCTL_CONFI_FILE "/etc/sysctl.d/10-sysctl-ssr.conf"
#define SYSCTL_ACCEPT_REDIRECTS_PATTERN "\"net.ipv(4|6).conf.*.accept_redirects\""
#define SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN "\"net.ipv(4|6).conf.*.accept_source_route\""

Sysctl::Sysctl()
{
    this->sysctl_config_ = ConfigPlain::create(SYSCTL_CONFI_FILE, "\\s*=\\s*", " = ");
}

std::vector<SysctlRedirect::SysctlVar> Sysctl::get_vars_by_pattern(const std::string &pattern)
{
    std::vector<SysctlVar> vars;
    std::string standard_output;
    std::string standard_error;
    std::vector<std::string> argv = {SYSCTL_COMMAND, "-a", "-r", pattern};

    RETURN_VAL_IF_TRUE(!Glib2Utils::spawn_sync(argv, &standard_output, &standard_error), vars);
    auto lines = StrUtils::split_lines(standard_output);
    for (const auto &line : lines)
    {
        auto fields = StrUtils::split_with_char(line, '=');
        CONTINUE_IF_TRUE(fields.size() != 2);
        vars.push_back(std::make_pair(fields[0], fields[1]));
    }
    return vars;
}

SysctlRedirect::SysctlRedirect()
{
}

bool SysctlRedirect::get(std::string &args, SSRErrorCode &error_code)
{
    auto redirect_vars = this->get_vars_by_pattern(SYSCTL_ACCEPT_REDIRECTS_PATTERN);
    RETURN_ERROR_IF_TRUE(redirect_vars.empty(), SSRErrorCode::ERROR_FAILED);

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
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

bool SysctlRedirect::set(const std::string &args, SSRErrorCode &error_code)
{
    auto redirect_vars = this->get_vars_by_pattern(SYSCTL_ACCEPT_REDIRECTS_PATTERN);
    RETURN_ERROR_IF_TRUE(redirect_vars.empty(), SSRErrorCode::ERROR_FAILED);

    try
    {
        auto values = StrUtils::str2json(args);
        RETURN_ERROR_IF_TRUE(!values[SYSCTL_JSON_KEY_ENABLED].isBool(), SSRErrorCode::ERROR_FAILED);
        auto enabled = values[SYSCTL_JSON_KEY_ENABLED].asBool();

        // 写入文件中
        for (const auto &var : redirect_vars)
        {
            RETURN_ERROR_IF_FALSE(this->sysctl_config_->set_value(var.first, enabled ? "1" : "0"), SSRErrorCode::ERROR_FAILED);
        }

        // 从文件中刷新
        std::vector<std::string> argv = {SYSCTL_COMMAND, "--load", SYSCTL_CONFI_FILE};
        RETURN_ERROR_IF_TRUE(!Glib2Utils::spawn_sync(argv, nullptr, nullptr), SSRErrorCode::ERROR_FAILED);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

SysctlSourceRoute::SysctlSourceRoute()
{
}

bool SysctlSourceRoute::get(std::string &args, SSRErrorCode &error_code)
{
    auto redirect_vars = this->get_vars_by_pattern(SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN);
    RETURN_ERROR_IF_TRUE(redirect_vars.empty(), SSRErrorCode::ERROR_FAILED);

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
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

bool SysctlSourceRoute::set(const std::string &args, SSRErrorCode &error_code)
{
    auto redirect_vars = this->get_vars_by_pattern(SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN);
    RETURN_ERROR_IF_TRUE(redirect_vars.empty(), SSRErrorCode::ERROR_FAILED);

    try
    {
        auto values = StrUtils::str2json(args);
        RETURN_ERROR_IF_TRUE(!values[SYSCTL_JSON_KEY_ENABLED].isBool(), SSRErrorCode::ERROR_FAILED);
        auto enabled = values[SYSCTL_JSON_KEY_ENABLED].asBool();

        // 写入文件中
        for (const auto &var : redirect_vars)
        {
            RETURN_ERROR_IF_FALSE(this->sysctl_config_->set_value(var.first, enabled ? "1" : "0"), SSRErrorCode::ERROR_FAILED);
        }

        // 从文件中刷新
        std::vector<std::string> argv = {SYSCTL_COMMAND, "--load", SYSCTL_CONFI_FILE};
        RETURN_ERROR_IF_TRUE(!Glib2Utils::spawn_sync(argv, nullptr, nullptr), SSRErrorCode::ERROR_FAILED);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        error_code = SSRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

}  // namespace Network
}  // namespace Kiran
