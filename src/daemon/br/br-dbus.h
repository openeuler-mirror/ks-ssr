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
#include "br-protocol.hxx"
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

    static BRDBus *getInstance()
    {
        return m_instance;
    };
    static void globalInit(QObject *parent);
    static void globalDeinit()
    {
        delete m_instance;
    };

public:  // PROPERTIES
    Q_PROPERTY(uint notification_status READ notificationStatus)
    uint notificationStatus() const;

    Q_PROPERTY(uint resource_monitor READ resourceMonitor)
    uint resourceMonitor() const;

    Q_PROPERTY(uint standard_type READ standardType)
    uint standardType() const;

    Q_PROPERTY(uint strategy_type READ strategyType)
    uint strategyType() const;

    Q_PROPERTY(uint time_scan READ timeScan)
    uint timeScan() const;

    Q_PROPERTY(QString version READ version)
    QString version() const;

    Q_PROPERTY(uint fallback_status READ fallbackStatus)
    uint fallbackStatus() const;

public Q_SLOTS:
    // 设置标准类型
    virtual void SetStandardType(const uint32_t &standardType);

    // 设置自定义加固标准
    virtual void ImportCustomRS(const QString &encodedStandard);

    // 设置策略类型
    virtual void SetStrategyType(const uint32_t &strategyType);

    // 设置定时扫描时间
    virtual void SetTimeScan(const uint32_t &timeScan);

    // 设置通知状态
    virtual void SetNotificationStatus(const uint32_t &notificationStatus);

    // 设置自定义加固策略
    virtual void ImportCustomRA(const QString &encodedStrategy);

    // 设置复选框状态
    virtual void SetCheckBox(const QString &reinforcementName, const bool &checkboxStatus);

    // 设置资源监控开关
    virtual void SetResourceMonitorSwitch(const uint32_t &resourceMonitor);

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
    virtual void SetReinforcement(const QString &reinforcementXML);

    // 重置指定的加固项
    virtual void ResetReinforcement(const QString &name);

    // 扫描指定加固项
    virtual void Scan(const QStringList &names);

    // 对加固项进行加固
    virtual void Reinforce(const QStringList &names);

    // 取消一个任务
    virtual void Cancel(const qlonglong &jobID);

    // 设置回退，回退到初始状态/回退到上一次的状态
    virtual void SetFallback(const uint32_t &snapshot);

    // 设置回退状态 退回进行中/回退未开始/回退完成
    virtual void SetFallbackStatus(const uint32_t &fallbackStatus);

    // 通知后端发生导出 加固策略/报表 操作，由后端记录日志。
    void ExportStrategy(bool);
    void GenerateReport(bool);

Q_SIGNALS:  // SIGNALS
    void CpuAverageLoadRatioHigher(const QString &ratio);
    void HomeFreeSpaceRatioLower(const QString &ratio);
    void ProgressFinished();
    void ReinforceProgress(const QString &progress);
    void RootFreeSpaceRatioLower(const QString &ratio);
    void ScanProgress(const QString &progress);
    void MemoryAbnormal(const QString &ratio);

private:
    void reinforce(const QDBusMessage &message, const QStringList &names);
    void setFallback(const QDBusMessage &message, const uint32_t &snapshot);

private:
    void init();
    // 扫描进度信号处理
    void scanResultHandle(const JobResult &jobResult);
    // 加固进度信号处理
    void reinforceResultHandle(const JobResult &jobResult);
    // 资源监控开启/关闭
    bool setResourceMonitor();
    // 进程完成处理函数
    void finishedScanProgress();
    // 加固完成处理函数
    void finishedReinforceProgress();

    void parseJsonParam(const Protocol::Reinforcement::ArgSequence &argSequence, QJsonObject &param);
    // 通过加固项名获取加固参数，返回值为json字符串
    QString getJsonParam(const QString &reinforceName);
    // 更新rh文件
    void updateRH(const QString &reinforceName, const QJsonObject &resultReturnValue);

    void homeFreeSpaceRatio(float spaceRatio);
    void rootFreeSpaceRatio(float spaceRatio);
    void cpuAverageLoadRatio(float loadRatio);
    void memoryRemainingRatio(float memoryRatio);

private:
    static BRDBus *m_instance;
    QTimer *m_resourceMonitorTimer;
    Configuration *m_configuration;
    Categories *m_categories;
    Plugins *m_plugins;
    ResourceMonitor *m_resourceMonitor;

    // 扫描任务
    QSharedPointer<Job> m_scanJob;
    // 加固任务
    QSharedPointer<Job> m_reinforceJob;

    // 激活信息
    QJsonValue m_licenseValues;

    // 加固前需要进行一次扫描，用于判断是否这次扫描是否为正常调用dbus接口的扫描
    bool m_isScanFlag;
    // 首次扫描的rh文件是否写入完成
    bool m_isFinishRHWrite;

    BRFallbackMethod m_fallbackMethod = BRFallbackMethod::BR_FALLBACK_METHOD_OTHER;
    BRAdaptor *m_dbus;
};
}  // namespace BRDaemon
}  // namespace KS
