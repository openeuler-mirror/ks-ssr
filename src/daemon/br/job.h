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
    JobResult() : job_id(0),
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
    int64_t getId() { return this->job_id_; };

    // 获取任务状态
    BRJobState getState() { return this->state_; };

    // 获取操作
    QSharedPointer<Operation> getOperation(int32_t operation_id) { return MapHelper::getValue(this->operations_, operation_id); };

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

    // sigc::signal<void, const JobResult &> &signal_process_changed() { return this->process_changed_; };

    // sigc::signal<void> &signal_process_finished() { return this->process_finished_; };

private:
    Job(int64_t job_id);

    // 运行前初始化
    void run_init();

    // 线程中运行操作
    void run_operation(QSharedPointer<Operation> operation);

    // 空闲时监听任务的进度
    bool idle_check_operation();

private:
    int64_t job_id_;
    BRJobState state_;
    QTimer *timer;
    // sigc::connection timeout_handler_;
    // 操作集合
    QMap<int32_t, QSharedPointer<Operation>> operations_;
    // 任务执行的结果
    JobResult job_result_;
    // 操作互斥
    QMutex operations_mutex_;
    // 任务需要取消
    bool need_cancel_;

    static int64_t job_count_;

    // sigc::signal<void, const JobResult &> process_changed_;
    // sigc::signal<void> process_finished_;

signals:
    // 任务进度变化的信号
    void process_changed_(const JobResult &);

    // 任务完成信号
    void process_finished_();
};

typedef QVector<QSharedPointer<Job>> BRJobVec;
}  // namespace BRDaemon
}  // namespace KS
