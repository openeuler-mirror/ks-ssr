/**
 * @file          /kiran-sse-manager/lib/dbus/dbus-systemd-proxy.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

namespace Kiran
{
class DBusSystemdProxy
{
public:
    DBusSystemdProxy();
    virtual ~DBusSystemdProxy(){};

    // 获取文件启动状态
    std::string get_unit_file_state(const std::string &unit_name);
    // 是否开机启动
    bool enable_unit_file(const std::string &unit_name, bool enabled);

    // 开启服务
    bool start_unit(const std::string &unit_name);
    // 关闭服务
    bool stop_unit(const std::string &unit_name);

    // 获取服务状态
    std::string get_unit_active_state(const std::string &unit_name);

private:
    Glib::VariantContainerBase call_manager_method(const std::string &method_name, const Glib::VariantContainerBase &parameters);
    bool call_manager_method_noresult(const std::string &method_name, const Glib::VariantContainerBase &parameters);

private:
    Glib::RefPtr<Gio::DBus::Proxy> systemd_proxy_;
};
}  // namespace Kiran
