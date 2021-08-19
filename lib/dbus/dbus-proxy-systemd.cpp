/**
 * @file          /kiran-ssr-manager/lib/dbus/dbus-proxy-systemd.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "lib/dbus/dbus-proxy-systemd.h"

namespace Kiran
{
#define SYSTEMD_DBUS_NAME "org.freedesktop.systemd1"
#define SYSTEMD_DBUS_OBJECT_PATH "/org/freedesktop/systemd1"
#define SYSTEMD_DBUS_INTERFACE_MANAGER "org.freedesktop.systemd1.Manager"
// SDMM: systemd dbus manager method
#define SDMM_GET_UNIT_FILE_STATE "GetUnitFileState"
#define SDMM_GET_UNIT "GetUnit"
#define SDMM_ENABLE_UNIT_FILES "EnableUnitFiles"
#define SDMM_DISABLE_UNIT_FILES "DisableUnitFiles"
#define SDMM_START_UNIT "StartUnit"
#define SDMM_STOP_UNIT "StopUnit"

#define SYSTEMD_DBUS_INTERFACE_UNIT "org.freedesktop.systemd1.Unit"
// SDUP: systemd dbus unit property
#define SDUP_ACTIVE_STATE "ActiveState"

DBusSystemdProxy::DBusSystemdProxy()
{
    this->systemd_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                 SYSTEMD_DBUS_NAME,
                                                                 SYSTEMD_DBUS_OBJECT_PATH,
                                                                 SYSTEMD_DBUS_INTERFACE_MANAGER);
}

std::string DBusSystemdProxy::get_unit_file_state(const std::string &unit_name)
{
    KLOG_PROFILE("unit name: %s.", unit_name.c_str());

    try
    {
        auto parameters = Glib::VariantContainerBase(g_variant_new("(s)", unit_name.c_str()));
        auto retval = this->call_manager_method(SDMM_GET_UNIT_FILE_STATE, parameters);
        RETURN_VAL_IF_TRUE(retval.get_n_children() == 0, std::string());
        return Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(retval.get_child()).get();
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
    }
    return std::string();
}

bool DBusSystemdProxy::enable_unit_file(const std::string &unit_name, bool enabled)
{
    try
    {
        auto method_name = enabled ? SDMM_ENABLE_UNIT_FILES : SDMM_DISABLE_UNIT_FILES;
        auto parameters = Glib::Variant<std::tuple<std::vector<Glib::ustring>, bool, bool>>::create(
            std::make_tuple(std::vector<Glib::ustring>{unit_name}, false, false));
        return this->call_manager_method_noresult(method_name, parameters);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
        return false;
    }
    return true;
}

std::string DBusSystemdProxy::get_unit_active_state(const std::string &unit_name)
{
    KLOG_PROFILE("unit name: %s.", unit_name.c_str());

    try
    {
        auto parameters = Glib::VariantContainerBase(g_variant_new("(s)", unit_name.c_str()));
        auto retval = this->call_manager_method(SDMM_GET_UNIT, parameters);
        RETURN_VAL_IF_TRUE(retval.get_n_children() == 0, std::string());
        auto unit_object_path = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(retval.get_child()).get();
        auto unit_proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                SYSTEMD_DBUS_NAME,
                                                                unit_object_path,
                                                                SYSTEMD_DBUS_INTERFACE_UNIT);

        Glib::VariantBase state;
        unit_proxy->get_cached_property(state, SDUP_ACTIVE_STATE);
        return Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(state).get();
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
    }
    return std::string();
}

bool DBusSystemdProxy::start_unit(const std::string &unit_name)
{
    KLOG_PROFILE("unit name: %s.", unit_name.c_str());

    auto parameters = Glib::VariantContainerBase(g_variant_new("(ss)", unit_name.c_str(), "replace"), false);
    return this->call_manager_method_noresult(SDMM_START_UNIT, parameters);
}

bool DBusSystemdProxy::stop_unit(const std::string &unit_name)
{
    KLOG_PROFILE("unit name: %s.", unit_name.c_str());

    auto parameters = Glib::VariantContainerBase(g_variant_new("(ss)", unit_name.c_str(), "replace"), false);
    return this->call_manager_method_noresult(SDMM_STOP_UNIT, parameters);
}

Glib::VariantContainerBase DBusSystemdProxy::call_manager_method(const std::string &method_name, const Glib::VariantContainerBase &parameters)
{
    KLOG_PROFILE("method_name: %s.", method_name.c_str());

    Glib::VariantContainerBase retval;
    try
    {
        retval = this->systemd_proxy_->call_sync(method_name, parameters);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to call systemd method %s: %s", method_name.c_str(), e.what().c_str());
    }
    return retval;
}

bool DBusSystemdProxy::call_manager_method_noresult(const std::string &method_name, const Glib::VariantContainerBase &parameters)
{
    KLOG_PROFILE("method_name: %s.", method_name.c_str());

    auto retval = this->call_manager_method(method_name, parameters);
    if (retval.gobj())
    {
        return true;
    }
    return false;
}
}  // namespace Kiran
