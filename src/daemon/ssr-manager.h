/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-manager.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <ssr_dbus_stub.h>
#include "src/daemon/ssr-job.h"
#include "ssr-config.h"

namespace Kiran
{
class SSRConfiguration;
class SSRCategories;
class SSRPlugins;
class SSRJob;

class SSRManager : public SSRStub
{
public:
    SSRManager();
    virtual ~SSRManager();

    static SSRManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    // 设置标准类型
    virtual void SetStandardType(guint32 standard_type, MethodInvocation &invocation);

    // 设置自定义加固标准
    virtual void ImportCustomRS(const Glib::ustring &encoded_standard, MethodInvocation &invocation);

    // 获取分类
    virtual void GetCategories(MethodInvocation &invocation);

    // 获取插件
    // virtual void GetPlugins(MethodInvocation &invocation);

    // 根据加固标准配置获取加固项
    virtual void GetReinforcements(MethodInvocation &invocation);

    // 设置自定义加固参数
    virtual void SetReinforcementArgs(const Glib::ustring &name, const Glib::ustring &custom_args, MethodInvocation &invocation);

    // 扫描指定加固项
    virtual void Scan(const Glib::ustring &scan_range, MethodInvocation &invocation);

    // 对加固项进行加固
    virtual void Reinforce(const Glib::ustring &reinforcements, MethodInvocation &invocation);

    // 取消一个任务
    virtual void Cancel(gint64 job_id, MethodInvocation &invocation);

    virtual bool version_setHandler(const Glib::ustring &value) { return true; };
    virtual bool standard_type_setHandler(guint32 value);

    virtual Glib::ustring version_get() { return PROJECT_VERSION; };
    virtual guint32 standard_type_get();

private:
    void init();

    // 扫描进度信号处理
    void on_scan_process_changed_cb(const SSRJobResult &job_result);
    // 加固进度信号处理
    void on_reinfoce_process_changed_cb(const SSRJobResult &job_result);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static SSRManager *instance_;

    SSRConfiguration *configuration_;
    SSRCategories *categories_;
    SSRPlugins *plugins_;

    // 扫描任务
    std::shared_ptr<SSRJob> scan_job_;

    // 加固任务
    std::shared_ptr<SSRJob> reinforce_job_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
};
}  // namespace Kiran