/**
 * @file          /kiran-ssr-manager/src/daemon/manager.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <ssr_dbus_stub.h>
#include "src/daemon/job.h"

namespace Kiran
{
namespace Daemon
{
class Configuration;
class Categories;
class Plugins;
class Job;

class Manager : public SSRStub
{
public:
    Manager();
    virtual ~Manager();

    static Manager *get_instance() { return instance_; };

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

    // 获取指定加固项的基本信息和加固参数
    virtual void GetReinforcement(const Glib::ustring &name, MethodInvocation &invocation);

    // 设置自定义加固参数
    virtual void SetReinforcement(const Glib::ustring &reinforcement_xml, MethodInvocation &invocation);

    // 扫描指定加固项
    virtual void Scan(const std::vector<Glib::ustring> &names, MethodInvocation &invocation);

    // 对加固项进行加固
    virtual void Reinforce(const std::vector<Glib::ustring> &names, MethodInvocation &invocation);

    // 取消一个任务
    virtual void Cancel(gint64 job_id, MethodInvocation &invocation);

    virtual bool version_setHandler(const Glib::ustring &value) { return true; };
    virtual bool standard_type_setHandler(guint32 value);

    virtual Glib::ustring version_get() { return PROJECT_VERSION; };
    virtual guint32 standard_type_get();

private:
    void init();

    // 获取加固项信息
    Json::Value get_reinforcement_json(const std::string &name);

    // 扫描进度信号处理
    void on_scan_process_changed_cb(const JobResult &job_result);
    // 加固进度信号处理
    void on_reinfoce_process_changed_cb(const JobResult &job_result);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static Manager *instance_;

    Configuration *configuration_;
    Categories *categories_;
    Plugins *plugins_;

    // 扫描任务
    std::shared_ptr<Job> scan_job_;

    // 加固任务
    std::shared_ptr<Job> reinforce_job_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
};
}  // namespace Daemon
}  // namespace Kiran