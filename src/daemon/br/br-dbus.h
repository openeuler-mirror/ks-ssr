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

#pragma once

#include <QDBusContext>
#include <QStringView>
#include <QTimer>
#include "job.h"
#include "resource-monitor.h"

class BRAdaptor;

namespace KS
{
namespace BRDaemon
{
class Configuration;
class Categories;
class Plugins;
class Job;
class LicenseObject;

class BRDBus : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    BRDBus(QObject *parent);
    // BRDBus(QObject *parent);
    virtual ~BRDBus();

    static BRDBus *getInstance() { return instance_; };

    static void globalInit(QObject *parentt);
    // static void globalInit(QObject* parent);

    static void globalDeinit() { delete instance_; };

public:  // PROPERTIES
    Q_PROPERTY(uint notification_status READ notification_status)
    uint notification_status() const;

    Q_PROPERTY(uint resource_monitor READ resource_monitor)
    uint resource_monitor() const;

    Q_PROPERTY(uint standard_type READ standard_type)
    uint standard_type() const;

    Q_PROPERTY(uint strategy_type READ strategy_type)
    uint strategy_type() const;

    Q_PROPERTY(uint time_scan READ time_scan)
    uint time_scan() const;

    Q_PROPERTY(QString version READ version)
    QString version() const;

public Q_SLOTS:
    // 设置标准类型
    virtual void SetStandardType(const uint32_t &standard_type);

    // 设置自定义加固标准
    virtual void ImportCustomRS(const QString &encoded_standard);

    // 设置策略类型
    virtual void SetStrategyType(const uint32_t &strategy_type);

    // 设置定时扫描时间
    virtual void SetTimeScan(const uint32_t &time_scan);

    // 设置通知状态
    virtual void SetNotificationStatus(const uint32_t &notification_status);

    // 设置自定义加固策略
    virtual void ImportCustomRA(const QString &encoded_strategy);

    // 设置复选框状态
    virtual void SetCheckBox(const QString &reinforcement_name, const bool &checkbox_status);

    // 设置资源监控开关
    virtual void SetResourceMonitorSwitch(const uint32_t &resource_monitor);

    // 获取分类
    virtual QString GetCategories();

    // 获取加固标准配置
    virtual QString GetRS();

    // 获取所有加固项的基本信息和加固参数
    virtual QString GetReinforcements();

    // 重置所有加固项的信息到默认值
    virtual void ResetReinforcements();

    // 获取指定加固项的基本信息和加固参数
    virtual QString GetReinforcement(const QString &name);

    // 设置自定义加固参数
    virtual void SetReinforcement(const QString &reinforcement_xml);

    // 重置指定的加固项
    virtual void ResetReinforcement(const QString &name);

    // 扫描指定加固项
    virtual qlonglong Scan(const QStringList &names);

    // 对加固项进行加固
    virtual qlonglong Reinforce(const QStringList &names);

    // 取消一个任务
    virtual void Cancel(const qlonglong &job_id);

    // 获取授权信息
    // virtual std::string GetLicense();

    virtual void SetFallback(const uint32_t &snapshot_status);

    // 通过激活码注册
    // virtual std::string ActivateByActivationCode(const std::string &activation_code);

    // 作用存疑，需要确认
    // virtual void on_get_property(::BRDBus::InterfaceAdaptor &interface, const std::string &property, ::BRDBus::Variant &value);
    // virtual void on_set_property(::BRDBus::InterfaceAdaptor &interface, const std::string &property, const ::BRDBus::Variant &value);
Q_SIGNALS:  // SIGNALS
    void CpuAverageLoadRatioHigher(const QString &ratio);
    void HomeFreeSpaceRatioLower(const QString &ratio);
    void ProgressFinished();
    void ReinforceProgress(const QString &progress);
    void RootFreeSpaceRatioLower(const QString &ratio);
    void ScanProgress(const QString &progress);
    void MemoryAbnormal(const QString &ratio);

private:
    void init();

    // 更新激活信息
    // void update_license_info();

    // 扫描进度信号处理
    void onScanProcessChangedCb(const JobResult &job_result);
    // 加固进度信号处理
    void onReinfoceProcessChangedCb(const JobResult &job_result);
    // 授权发生变化
    // void on_license_info_changed_cb(bool placeholder);
    // 资源监控开启/关闭
    bool onResourceMonitor();
    // 进程完成处理函数
    void scanProgressFinished()
    {
        is_scan_flag_ = true;
        emit ProgressFinished();
    };
    // 加固完成处理函数
    void reinfoceProgressFinished()
    {
        is_reinfoce_flag_ = true;
        is_scan_flag_ = true;
        emit ProgressFinished();
    };

    void homeFreeSpaceRatio(float spaceRatio);
    void rootFreeSpaceRatio(float spaceRatio);
    void cpuAverageLoadRatio(float loadRatio);
    void memoryRemainingRatio(float memoryRatio);

private:
    static BRDBus *instance_;

    // ::BRDBus::Connection dbus_connection_;

    QTimer *timer;
    // sigc::connection timeout_handler_;

    Configuration *configuration_;
    Categories *categories_;
    Plugins *plugins_;
    ResourceMonitor *resource_monitor_;

    // 扫描任务
    QSharedPointer<Job> scan_job_;
    // 加固任务
    QSharedPointer<Job> reinforce_job_;

    // 激活信息
    QJsonValue license_values;

    // 首次加固 全盘扫描
    bool is_frist_reinfoce_ = true;          // 是否首次加固
    bool is_frist_reinfoce_finish_ = false;  // 首次加固是否完成

    bool is_scan_flag_ = true;
    bool is_reinfoce_flag_ = true;

    BRSnapshotStatus snapshot_status_ = BRSnapshotStatus::BR_SNAPSHOT_STATUS_OTHER;
    BRAdaptor *m_dbus;
};
}  // namespace BRDaemon
}  // namespace KS
