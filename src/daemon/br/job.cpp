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

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <QMutexLocker>
#include <QSharedPointer>
#include <QTimer>

#include "job.h"
#include "plugins.h"

namespace KS
{
namespace BRDaemon
{
int64_t Job::job_count_ = 0;

QSharedPointer<Job> Job::create()
{
    QSharedPointer<Job> job(new Job(++Job::job_count_));
    return job;
}

Job::Job(int64_t job_id)
    : job_id_(job_id),
      state_(BRJobState::BR_JOB_STATE_IDLE),
      timer(nullptr),
      need_cancel_(false)
{
}

Job::~Job()
{
    if (this->timer)
    {
        // this->timeout_handler_.disconnect();
        QObject::disconnect(this->timer, &QTimer::timeout, this, &Job::idle_check_operation);
        delete this->timer;
    }
}

QSharedPointer<Operation> Job::addOperation(const QString &plugin_name,
                                            const QString &reinforcement_name,
                                            std::function<QString(void)> func)
{
    auto operation = QSharedPointer<Operation>(new Operation({this->job_id_,
                                                              this->operations_.size() + 1,
                                                              plugin_name,
                                                              reinforcement_name,
                                                              std::move(func)}));
    if (this->operations_.find(operation->operation_id) != this->operations_.end())
    {
        // 正常不应该执行到这里
        KLOG_WARNING("The operation %d is already exist.", operation->operation_id);
        return QSharedPointer<Operation>();
    }
    this->operations_[operation->operation_id] = operation;
    return operation;
}

bool Job::runSync()
{
    KLOG_DEBUG("job id: %ld, operation num: %d.", this->job_id_, this->operations_.size());

    RETURN_VAL_IF_FALSE(this->state_ == BRJobState::BR_JOB_STATE_IDLE, false);
    this->state_ = BRJobState::BR_JOB_STATE_RUNNING;

    this->run_init();

    for (auto iter = this->operations_.begin(); iter != this->operations_.end(); ++iter)
    {
        OperationResult result;
        auto operation = iter.value();
        result.operation_id = operation->operation_id;
        result.result = operation->func();
        ++this->job_result_.finished_operation_num;
        this->job_result_.current_finished_operations.push_back(std::move(result));

        if (this->job_result_.finished_operation_num == this->job_result_.sum_operation_num)
            this->state_ = BRJobState::BR_JOB_STATE_DONE;
        Q_EMIT this->process_changed_(this->job_result_);
        if (this->job_result_.finished_operation_num == this->job_result_.sum_operation_num)
            Q_EMIT this->process_finished_();
    }
    this->state_ = BRJobState::BR_JOB_STATE_IDLE;
    return true;
}

bool Job::runAsync()
{
    KLOG_DEBUG("job id: %ld, operation num: %d.", this->job_id_, this->operations_.size());

    RETURN_VAL_IF_FALSE(this->state_ == BRJobState::BR_JOB_STATE_IDLE, false);
    this->state_ = BRJobState::BR_JOB_STATE_RUNNING;

    this->run_init();

    // 定时监听任务完成的情况
    timer = new QTimer();
    timer->setInterval(100);
    connect(this->timer, &QTimer::timeout, this, &Job::idle_check_operation);
    this->timer->start();
    auto &thread_pool = Plugins::getInstance()->getThreadPool();
    {
        QMutexLocker guard(&(this->operations_mutex_));
        for (auto iter = this->operations_.begin(); iter != this->operations_.end(); ++iter)
        {
            thread_pool.enqueueByIdx(std::hash<std::string>()(iter.value()->reinforcement_name.toStdString()),
                                     std::bind(&Job::run_operation, this, iter.value()));
        }
    }
    return true;
}

bool Job::cancel()
{
    // 只有在运行中的任务才可以取消
    RETURN_VAL_IF_FALSE(this->state_ == BRJobState::BR_JOB_STATE_RUNNING, false);
    this->need_cancel_ = true;
    return true;
}

void Job::run_init()
{
    this->job_result_.job_id = this->job_id_;
    this->job_result_.sum_operation_num = this->operations_.size();
    this->job_result_.finished_operation_num = 0;
    this->job_result_.running_operations.clear();
    this->job_result_.current_finished_operations.clear();
    this->need_cancel_ = false;
}

void Job::run_operation(QSharedPointer<Operation> operation)
{
    RETURN_IF_FALSE(operation);

    OperationResult result;
    result.operation_id = operation->operation_id;

    KLOG_DEBUG("running operation: %d, job id: %lu.", operation->operation_id, operation->job_id);

    {
        QMutexLocker guard(&(this->operations_mutex_));
        this->job_result_.running_operations.push_back(operation->operation_id);
        this->job_result_.queue_is_changed = true;
    }

    // 任务如果已经取消，则不再执行插件函数
    if (!this->need_cancel_)
    {
        result.result = operation->func();
    }

    KLOG_DEBUG("finished operation: %d, job id: %ld.", operation->operation_id, operation->job_id);

    {
        QMutexLocker guard(&(this->operations_mutex_));
        ++this->job_result_.finished_operation_num;
        // 从正在运行的队列中删除
        auto iter = std::remove(this->job_result_.running_operations.begin(), this->job_result_.running_operations.end(), operation->operation_id);
        this->job_result_.running_operations.erase(iter, this->job_result_.running_operations.end());
        // 添加到已完成队列
        this->job_result_.current_finished_operations.push_back(std::move(result));
        this->job_result_.queue_is_changed = true;
    }
}

bool Job::idle_check_operation()
{
    JobResult tmp_result;
    {
        QMutexLocker guard(&(this->operations_mutex_));
        if (this->job_result_.queue_is_changed)
        {
            tmp_result = this->job_result_;
            this->job_result_.current_finished_operations.clear();
            this->job_result_.queue_is_changed = false;
        }
    }

    // 只有任务发生变化才进行处理
    if (tmp_result.queue_is_changed)
    {
        KLOG_DEBUG("sum num: %d, finished num: %d, running num: %d, current finished num: %d.",
                   tmp_result.sum_operation_num,
                   tmp_result.finished_operation_num,
                   tmp_result.running_operations.size(),
                   tmp_result.current_finished_operations.size());

        if (tmp_result.finished_operation_num == tmp_result.sum_operation_num)
            this->state_ = this->need_cancel_ ? BRJobState::BR_JOB_STATE_CANCEL_DONE : BRJobState::BR_JOB_STATE_DONE;

        Q_EMIT this->process_changed_(tmp_result);
        if (tmp_result.finished_operation_num == tmp_result.sum_operation_num)
        {
            Q_EMIT this->process_finished_();
        }

        // 确保前面的信号发送出去后再将状态设置为空闲，这样信号的回调函数能够收到正确的状态值
        if (this->state_ == BRJobState::BR_JOB_STATE_DONE ||
            this->state_ == BRJobState::BR_JOB_STATE_CANCEL_DONE)
        {
            this->state_ = BRJobState::BR_JOB_STATE_IDLE;
            return false;
        }
    }
    return true;
}
}  // namespace BRDaemon
}  // namespace KS
