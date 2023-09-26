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

#include "plugins/cpp/network/reinforcements/firewalld.h"

namespace KS
{
namespace Network
{
#define FIREWALLD_UNIT_NAME "firewalld.service"
#define FIREWALLD_CMD_COMMAND "/usr/bin/firewall-cmd"

#define FIREWALLD_SWITCH_KEY_ENABLED "enabled"

#define FIREWALLD_ICMP_BLOCK_TIMESTAMP_REQUEST "timestamp-request"
#define FIREWALLD_ICMP_BLOCK_KEY_TIMESTAMP_REQUEST "disable_timestamp_request"

FirewalldSwitch::FirewalldSwitch()
{
    this->systemd_proxy_ = std::make_shared<DBusSystemdProxy>();
}

bool FirewalldSwitch::get(std::string &args, BRErrorCode &error_code)
{
    Json::Value values;

    try
    {
        // 开机自动启动
        // auto unit_file_state = this->systemd_proxy_->get_unit_file_state(FIREWALLD_UNIT_NAME);
        // values[FIREWALLD_SWITCH_KEY_ENABLED] = (unit_file_state == "enabled");

        // 是否运行
        auto state_str = this->systemd_proxy_->get_unit_active_state(FIREWALLD_UNIT_NAME);
        values[FIREWALLD_SWITCH_KEY_ENABLED] = (state_str == "active" || state_str == "activating");

        args = StrUtils::json2str(values);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

bool FirewalldSwitch::set(const std::string &args, BRErrorCode &error_code)
{
    try
    {
        Json::Value values = StrUtils::str2json(args);
        if (values[FIREWALLD_SWITCH_KEY_ENABLED].isBool())
        {
            auto enabled = values[FIREWALLD_SWITCH_KEY_ENABLED].asBool();
            this->systemd_proxy_->enable_unit_file(FIREWALLD_UNIT_NAME, enabled);
            if (enabled)
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
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
}

FirewalldICMPTimestamp::FirewalldICMPTimestamp()
{
}

bool FirewalldICMPTimestamp::get(std::string &args, BRErrorCode &error_code)
{
    std::string standard_output;
    Json::Value values;
    std::vector<std::string> argv = {FIREWALLD_CMD_COMMAND, "--list-icmp-blocks"};
    RETURN_ERROR_IF_TRUE(!MiscUtils::spawn_sync(argv, &standard_output), BRErrorCode::ERROR_FAILED);
    auto icmp_blocks = StrUtils::split_with_char(standard_output, ' ', true);
    auto iter = std::find(icmp_blocks.begin(), icmp_blocks.end(), FIREWALLD_ICMP_BLOCK_TIMESTAMP_REQUEST);

    try
    {
        values[FIREWALLD_ICMP_BLOCK_KEY_TIMESTAMP_REQUEST] = bool(iter != icmp_blocks.end());
        args = StrUtils::json2str(values);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

bool FirewalldICMPTimestamp::set(const std::string &args, BRErrorCode &error_code)
{
    try
    {
        std::string standard_output;

        auto values = StrUtils::str2json(args);
        RETURN_ERROR_IF_FALSE(values[FIREWALLD_ICMP_BLOCK_KEY_TIMESTAMP_REQUEST].isBool(), BRErrorCode::ERROR_FAILED);
        auto timestamp_request = values[FIREWALLD_ICMP_BLOCK_KEY_TIMESTAMP_REQUEST].asBool();

        // 持久化存储，该命令不会更新运行时配置
        auto operation = fmt::format("--{0}-icmp-block=" FIREWALLD_ICMP_BLOCK_TIMESTAMP_REQUEST, timestamp_request ? "add" : "remove");
        std::vector<std::string> permanet_argv = {FIREWALLD_CMD_COMMAND, operation, "--permanent"};
        RETURN_ERROR_IF_FALSE(MiscUtils::spawn_sync(permanet_argv), BRErrorCode::ERROR_FAILED);

        // 重新加载，让持久化配置立即生效
        std::vector<std::string> reload_argv = {FIREWALLD_CMD_COMMAND, "--reload"};
        RETURN_ERROR_IF_FALSE(MiscUtils::spawn_sync(reload_argv), BRErrorCode::ERROR_FAILED);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        error_code = BRErrorCode::ERROR_FAILED;
        return false;
    }
    return true;
}

}  // namespace Network
}  // namespace KS
