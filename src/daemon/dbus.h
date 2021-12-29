/**
 * @file          /kiran-ssr-manager/src/daemon/dbus.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <ssr_dbus_stub.h>
#include "src/daemon/job.h"

#include "license_manager_dbus_proxy.h"
#include "license_object_dbus_proxy.h"

namespace Kiran
{
namespace Daemon
{
class Configuration;
class Categories;
class Plugins;
class Job;
class LicenseObject;

class DBus : public SSRStub
{
public:
    DBus();
    virtual ~DBus();

    static DBus *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    // 设置标准类型
    virtual void SetStandardType(guint32 standard_type, MethodInvocation &invocation);

    // 设置自定义加固标准
    virtual void ImportCustomRS(const Glib::ustring &encoded_standard, MethodInvocation &invocation);

    // 获取分类
    virtual void GetCategories(MethodInvocation &invocation);

    // 获取加固标准配置
    virtual void GetRS(MethodInvocation &invocation);

    // 获取所有加固项的基本信息和加固参数
    virtual void GetReinforcements(MethodInvocation &invocation);

    // 重置所有加固项的信息到默认值
    virtual void ResetReinforcements(MethodInvocation &invocation);

    // 获取指定加固项的基本信息和加固参数
    virtual void GetReinforcement(const Glib::ustring &name, MethodInvocation &invocation);

    // 设置自定义加固参数
    virtual void SetReinforcement(const Glib::ustring &reinforcement_xml, MethodInvocation &invocation);

    // 重置指定的加固项
    virtual void ResetReinforcement(const Glib::ustring &name, MethodInvocation &invocation);

    // 扫描指定加固项
    virtual void Scan(const std::vector<Glib::ustring> &names, MethodInvocation &invocation);

    // 对加固项进行加固
    virtual void Reinforce(const std::vector<Glib::ustring> &names, MethodInvocation &invocation);

    // 取消一个任务
    virtual void Cancel(gint64 job_id, MethodInvocation &invocation);

    // 获取授权信息
    virtual void GetLicense(MethodInvocation &invocation);

    // 通过激活码注册
    virtual void ActivateByActivationCode(const Glib::ustring &activation_code, MethodInvocation &invocation);

    virtual bool version_setHandler(const Glib::ustring &value) { return true; };
    virtual bool standard_type_setHandler(guint32 value);

    virtual Glib::ustring version_get() { return PROJECT_VERSION; };
    virtual guint32 standard_type_get();

private:
    void init();

    // 更新激活信息
    void update_license_info();

    // 扫描进度信号处理
    void on_scan_process_changed_cb(const JobResult &job_result);
    // 加固进度信号处理
    void on_reinfoce_process_changed_cb(const JobResult &job_result);
    // 授权发生变化
    void on_license_info_changed_cb(bool placeholder);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static DBus *instance_;

    Configuration *configuration_;
    Categories *categories_;
    Plugins *plugins_;

    // 扫描任务
    std::shared_ptr<Job> scan_job_;
    // 加固任务
    std::shared_ptr<Job> reinforce_job_;

    Glib::RefPtr<Kiran::LicenseManagerProxy> license_manager_proxy_;
    Glib::RefPtr<Kiran::LicenseObjectProxy> license_object_proxy_;

    // 激活信息
    Json::Value license_values;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
};
}  // namespace Daemon
}  // namespace Kiran