/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#pragma once

#include <QObject>
#include "br-protocol.hxx"
#include "job.h"

namespace KS
{
namespace BR
{
class Configuration;
class Plugins;

class JobDispatcher : public QObject
{
    Q_OBJECT

public:
    JobDispatcher(Configuration *configuration,
                  Plugins *plugins,
                  QObject *parent = nullptr);
    virtual ~JobDispatcher(){};

    void init();

public:
    // 全量扫描
    bool scanAll();
    // 扫描指定加固项
    bool scan(const QStringList &names);
    // 获取当前扫描结果，只有在扫描中才会获取到信息，主要用于网络版断线重连时可以获取到当前扫描中的详细信息
    Protocol::JobResult getScanResult();
    // 全量加固
    bool reinforceAll();
    // 对加固项进行加固
    bool reinforce(const QStringList &names);
    // 获取当前加固结果
    Protocol::JobResult getReinforceResult();
    // 回退
    bool rollback(int method);
    // 获取调度状态
    BRDispatchState getState();
    // 取消一个任务
    bool cancel(const qlonglong &jobID);

Q_SIGNALS:
    // 扫描进度
    void scanProgress(const QString &progress);
    // 扫描完成
    void scanFinished();
    // 加固进度
    void reinforceProgress(const QString &progress);
    // 加固完成
    void reinforceFinished();
    // 回退进度
    void rollbackProgress(const QString &progress);
    // 回退完成
    void rollbackFinished();

private:
    // 备份指定加固项
    bool backup(const QStringList &names);

private:
    void initJobResult(const QStringList &names);
    // 结果更新到缓存
    void cacheResult(const Protocol::ReinforcementResult &reinforcementResult);
    // 在加固前保存系统历史记录便于回退
    void saveHistoryBeforeReinforce(const QStringList &reinforcementNames, const QString &savePath);
    // 启动加固
    bool startReinforce(const QStringList &names);
    // 扫描进度信号处理
    void processScanProgress(const JobResult &jobResult);
    // 加固进度信号处理
    void processReinforceProgress(const JobResult &jobResult);
    // 回退进度信号处理
    void processFallbackProgress(const QString &progress);
    // 进程完成处理函数
    void processScanFinished();
    // 加固完成处理函数
    void processReinforceFinished();
    // 回退完成处理函数
    void processFallbackFinished();
    // 将加固参数xml转json字符串
    QString reinforcementArgXml2Str(const KS::Protocol::Reinforcement::ArgSequence &args);

private:
    static JobManager *m_instance;
    Configuration *m_configuration;
    Plugins *m_plugins;
    // 扫描任务
    QSharedPointer<Job> m_scanJob;
    // 记录当前扫描过程中完整的扫描结果，而不是发送信号这一次的结果
    Protocol::JobResult m_scanJobResult;
    // 加固任务
    QSharedPointer<Job> m_reinforceJob;
    // 记录当前加固过程中完整的结果，而不是发送信号这一次的结果
    Protocol::JobResult m_reinforceJobResult;
    // 是否处于加固中
    bool isReinforce;
    // 是否处于回退操作中，因为回退也是调用的加固函数，所以需要这里需要用标记位区分是否是回退
    bool isFallback;
    //
    QMetaObject::Connection m_scanBeforeReinforceConnection;
};
}  // namespace BR
}  // namespace KS