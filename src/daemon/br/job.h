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

#include <QMutex>
#include <QTimer>
#include "lib/base/base.h"

namespace KS
{
namespace BRDaemon
{
struct OperationResult
{
    // 操作ID
    int32_t operation_id;
    // 操作结果
    QString result;
};
typedef QVector<QSharedPointer<OperationResult>> OperationResultVec;

struct Operation
{
    // 任务ID
    int64_t job_id;
    // 操作ID
    int32_t operation_id;
    // 插件名
    QString plugin_name;
    // 加固项名
    QString reinforcement_name;
    // 执行的函数
    std::function<QString(void)> func;
};
typedef QVector<QSharedPointer<Operation>> OperationVec;

// 任务返回的结果
struct JobResult
{
    JobResult()
        : job_id(0),
          sum_operation_num(0),
          finished_operation_num(0),
          queue_is_changed(false){};

    // 任务ID
    int64_t job_id;
    // 总操作数
    int32_t sum_operation_num;
    // 已经完成的操作数
    int32_t finished_operation_num;
    // 队列发生变化
    bool queue_is_changed;
    // 正在运行的操作
    QVector<int32_t> running_operations;
    // 从上一次发送信号到这一次完成的操作
    // QVector<int32_t> current_finished_operations;
    QVector<OperationResult> current_finished_operations;
};

// TODO: 同一个加固项放在同一个线程执行
class Job : public QObject
{
    Q_OBJECT
public:
    virtual ~Job();
    // 创建任务
    static QSharedPointer<Job> create();
    // 获取任务ID
    int64_t getId()
    {
        return this->m_jobID;
    };
    // 获取任务状态
    BRJobState getState()
    {
        return this->m_state;
    };
    // 获取操作
    QSharedPointer<Operation> getOperation(int32_t operationID)
    {
        return MapHelper::getValue(this->m_operations, operationID);
    };
    // 添加一个操作，返回操作ID
    QSharedPointer<Operation> addOperation(const QString &plugin_name,
                                           const QString &reinforcement_name,
                                           std::function<QString(void)> func);

    // 同步运行任务
    bool runSync();
    // 异步运行任务
    bool runAsync();
    // 取消任务，只对run_async有效
    bool cancel();

private:
    Job(int64_t job_id);
    // 运行前初始化
    void runInit();
    // 线程中运行操作
    void runOperation(QSharedPointer<Operation> operation);
    // 空闲时监听任务的进度
    bool idleCheckOperation();

private:
    int64_t m_jobID;
    BRJobState m_state;
    // 定时监控任务完成状态
    QTimer *m_monitorTimer;
    // 操作集合
    QMap<int32_t, QSharedPointer<Operation>> m_operations;
    // 任务执行的结果
    JobResult m_jobResult;
    // 操作互斥
    QMutex m_operationsMutex;
    // 任务需要取消
    bool m_isNeedCancel;
    static int64_t m_jobCount;

signals:
    // 任务进度变化的信号
    void processChanged(const JobResult &);

    // 任务完成信号
    void processFinished();
};

typedef QVector<QSharedPointer<Job>> BRJobVec;
}  // namespace BRDaemon
}  // namespace KS
