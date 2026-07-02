/**
 * @file          /ks-ssr-manager/src/daemon/dbus.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "src/daemon/job.h"
#include "src/daemon/resource-monitor.h"

#include "license_manager_dbus_proxy.h"
#include "license_object_dbus_proxy.h"
#include "ssr_dbus_stub.h"

namespace KS
{
namespace Daemon
{
class Configuration;
class Categories;
class Plugins;
class Job;
class LicenseObject;

class DBus : public com::kylinsec::SSR_adaptor,
             public ::DBus::IntrospectableAdaptor,
             public ::DBus::ObjectAdaptor,
             public ::DBus::PropertiesAdaptor
{
    class LicenseManagerProxy : public com::kylinsec::Kiran::LicenseManager_proxy,
                                public ::DBus::IntrospectableProxy,
                                public ::DBus::ObjectProxy
    {
    public:
        LicenseManagerProxy(::DBus::Connection &connection,
                            const std::string &path,
                            const std::string &service_name) : ::DBus::ObjectProxy(connection, path, service_name.c_str()){};
        virtual ~LicenseManagerProxy(){};
    };

    class LicenseObjectProxy : public com::kylinsec::Kiran::LicenseObject_proxy,
                               public ::DBus::IntrospectableProxy,
                               public ::DBus::ObjectProxy
    {
    public:
        LicenseObjectProxy(::DBus::Connection &connection,
                           const std::string &path,
                           const std::string &service_name) : ::DBus::ObjectProxy(connection, path, service_name.c_str()){};
        virtual ~LicenseObjectProxy(){};

        sigc::signal<void, bool> signal_license_changed() { return this->license_changed_; };

    protected:
        virtual void LicenseChanged(const bool &placeholder) { this->license_changed_.emit(placeholder); };

    private:
        sigc::signal<void, bool> license_changed_;
    };

public:
    DBus(::DBus::Connection &connection);
    virtual ~DBus();

    static DBus *get_instance() { return instance_; };

    static void global_init(::DBus::Connection &connection);

    static void global_deinit() { delete instance_; };

protected:
    // 设置标准类型
    virtual void SetStandardType(const uint32_t &standard_type);

    // 设置自定义加固标准
    virtual void ImportCustomRS(const std::string &encoded_standard);

    // 设置策略类型
    virtual void SetStrategyType(const uint32_t &strategy_type);

    // 设置自定义加固策略
    virtual void ImportCustomRA(const std::string &encoded_strategy);

    // 设置复选框状态
    virtual void SetCheckBox(const std::string &reinforcement_name, const bool &checkbox_status);

    // 获取分类
    virtual std::string GetCategories();

    // 获取加固标准配置
    virtual std::string GetRS();

    // 获取所有加固项的基本信息和加固参数
    virtual std::string GetReinforcements();

    // 重置所有加固项的信息到默认值
    virtual void ResetReinforcements();

    // 获取指定加固项的基本信息和加固参数
    virtual std::string GetReinforcement(const std::string &name);

    // 设置自定义加固参数
    virtual void SetReinforcement(const std::string &reinforcement_xml);

    // 重置指定的加固项
    virtual void ResetReinforcement(const std::string &name);

//    std::string reinforcement_interface_fun();
    // 扫描指定加固项
    virtual int64_t Scan(const std::vector<std::string> &names);

    // 对加固项进行加固
    virtual int64_t Reinforce(const std::vector<std::string> &names);

    // 取消一个任务
    virtual void Cancel(const int64_t &job_id);

    // 获取授权信息
    virtual std::string GetLicense();

    virtual void SetFallback(const uint32_t &snapshot_status);

    // 通过激活码注册
    virtual void ActivateByActivationCode(const std::string &activation_code);

    virtual void on_get_property(::DBus::InterfaceAdaptor &interface, const std::string &property, ::DBus::Variant &value);
    virtual void on_set_property(::DBus::InterfaceAdaptor &interface, const std::string &property, const ::DBus::Variant &value);

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
    // 资源监控开启/关闭
    bool on_resource_monitor();
    // 进程完成处理函数
    void scan_progress_finished() { is_scan_flag_ = true; };
    // 加固完成处理函数
    void reinfoce_progress_finished() { is_reinfoce_flag_ = true; is_scan_flag_ = true; this->ProgressFinished();};

    void homeFreeSpaceRatio(const float space_ratio);
    void rootFreeSpaceRatio(const float space_ratio);
    void cpuAverageLoadRatio(const float load_ratio);
    void vmstatSiSo(const std::vector<std::string> results);

private:
    static DBus *instance_;

    ::DBus::Connection dbus_connection_;

    sigc::connection timeout_handler_;

    Configuration *configuration_;
    Categories *categories_;
    Plugins *plugins_;
    ResourceMonitor *resource_monitor_;

    // 扫描任务
    std::shared_ptr<Job> scan_job_;
    // 加固任务
    std::shared_ptr<Job> reinforce_job_;

    std::shared_ptr<LicenseManagerProxy> license_manager_proxy_;
    std::shared_ptr<LicenseObjectProxy> license_object_proxy_;

    // 激活信息
    Json::Value license_values;

    // 首次加固 全盘扫描
    bool is_frist_reinfoce_ = true; //是否首次加固
    bool is_frist_reinfoce_finish_ = false; //首次加固是否完成

    bool is_scan_flag_ = true;
    bool is_reinfoce_flag_ = true;

    SSRSnapshotStatus snapshot_status_ = SSR_OTHER_STATUS;
};
}  // namespace Daemon
}  // namespace KS
