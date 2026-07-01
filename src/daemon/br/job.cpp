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
int64_t Job::m_jobCount = 0;

QSharedPointer<Job> Job::create()
{
    QSharedPointer<Job> job(new Job(++Job::m_jobCount));
    return job;
}

Job::Job(int64_t job_id)
    : m_jobID(job_id),
      m_state(BRJobState::BR_JOB_STATE_IDLE),
      m_monitorTimer(nullptr),
      m_isNeedCancel(false)
{
}

Job::~Job()
{
    if (this->m_monitorTimer)
    {
        // this->timeout_handler_.disconnect();
        QObject::disconnect(this->m_monitorTimer, &QTimer::timeout, this, &Job::idleCheckOperation);
        delete this->m_monitorTimer;
    }
}

QSharedPointer<Operation> Job::addOperation(const QString &plugin_name,
                                            const QString &reinforcement_name,
                                            std::function<QString(void)> func)
{
    auto operation = QSharedPointer<Operation>(new Operation({this->m_jobID,
                                                              this->m_operations.size() + 1,
                                                              plugin_name,
                                                              reinforcement_name,
                                                              std::move(func)}));
    if (this->m_operations.find(operation->operation_id) != this->m_operations.end())
    {
        // 正常不应该执行到这里
        KLOG_WARNING("The operation %d is already exist.", operation->operation_id);
        return QSharedPointer<Operation>();
    }
    this->m_operations[operation->operation_id] = operation;
    return operation;
}

bool Job::runSync()
{
    KLOG_DEBUG("job id: %ld, operation num: %d.", this->m_jobID, this->m_operations.size());

    RETURN_VAL_IF_FALSE(this->m_state == BRJobState::BR_JOB_STATE_IDLE, false);
    this->m_state = BRJobState::BR_JOB_STATE_RUNNING;

    this->runInit();

    for (auto iter = this->m_operations.begin(); iter != this->m_operations.end(); ++iter)
    {
        OperationResult result;
        auto operation = iter.value();
        result.operation_id = operation->operation_id;
        result.result = operation->func();
        ++this->m_jobResult.finished_operation_num;
        this->m_jobResult.current_finished_operations.push_back(std::move(result));

        if (this->m_jobResult.finished_operation_num == this->m_jobResult.sum_operation_num)
            this->m_state = BRJobState::BR_JOB_STATE_DONE;
        Q_EMIT this->processChanged(this->m_jobResult);
        if (this->m_jobResult.finished_operation_num == this->m_jobResult.sum_operation_num)
            Q_EMIT this->processFinished();
    }
    this->m_state = BRJobState::BR_JOB_STATE_IDLE;
    return true;
}

bool Job::runAsync()
{
    KLOG_DEBUG("job id: %ld, operation num: %d.", this->m_jobID, this->m_operations.size());

    RETURN_VAL_IF_FALSE(this->m_state == BRJobState::BR_JOB_STATE_IDLE, false);
    this->m_state = BRJobState::BR_JOB_STATE_RUNNING;

    this->runInit();

    // 定时监听任务完成的情况
    m_monitorTimer = new QTimer(this);
    m_monitorTimer->setInterval(100);
    connect(this->m_monitorTimer, &QTimer::timeout, this, &Job::idleCheckOperation);
    this->m_monitorTimer->start();
    auto &thread_pool = Plugins::getInstance()->getThreadPool();
    {
        QMutexLocker guard(&(this->m_operationsMutex));
        for (auto iter = this->m_operations.begin(); iter != this->m_operations.end(); ++iter)
        {
            thread_pool.enqueueByIdx(std::hash<std::string>()(iter.value()->reinforcement_name.toStdString()),
                                     std::bind(&Job::runOperation, this, iter.value()));
        }
    }
    return true;
}

bool Job::cancel()
{
    // 只有在运行中的任务才可以取消
    RETURN_VAL_IF_FALSE(this->m_state == BRJobState::BR_JOB_STATE_RUNNING, false);
    this->m_isNeedCancel = true;
    return true;
}

void Job::runInit()
{
    this->m_jobResult.job_id = this->m_jobID;
    this->m_jobResult.sum_operation_num = this->m_operations.size();
    this->m_jobResult.finished_operation_num = 0;
    this->m_jobResult.running_operations.clear();
    this->m_jobResult.current_finished_operations.clear();
    this->m_isNeedCancel = false;
}

void Job::runOperation(QSharedPointer<Operation> operation)
{
    RETURN_IF_FALSE(operation);

    OperationResult result;
    result.operation_id = operation->operation_id;

    KLOG_DEBUG("running operation: %d, job id: %lu.", operation->operation_id, operation->job_id);

    {
        QMutexLocker guard(&(this->m_operationsMutex));
        this->m_jobResult.running_operations.push_back(operation->operation_id);
        this->m_jobResult.queue_is_changed = true;
    }

    // 任务如果已经取消，则不再执行插件函数
    if (!this->m_isNeedCancel)
    {
        result.result = operation->func();
    }

    KLOG_DEBUG("finished operation: %d, job id: %ld.", operation->operation_id, operation->job_id);

    {
        QMutexLocker guard(&(this->m_operationsMutex));
        ++this->m_jobResult.finished_operation_num;
        // 从正在运行的队列中删除
        auto iter = std::remove(this->m_jobResult.running_operations.begin(), this->m_jobResult.running_operations.end(), operation->operation_id);
        this->m_jobResult.running_operations.erase(iter, this->m_jobResult.running_operations.end());
        // 添加到已完成队列
        this->m_jobResult.current_finished_operations.push_back(std::move(result));
        this->m_jobResult.queue_is_changed = true;
    }
}

bool Job::idleCheckOperation()
{
    JobResult tmp_result;
    {
        QMutexLocker guard(&(this->m_operationsMutex));
        if (this->m_jobResult.queue_is_changed)
        {
            tmp_result = this->m_jobResult;
            this->m_jobResult.current_finished_operations.clear();
            this->m_jobResult.queue_is_changed = false;
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
            this->m_state = this->m_isNeedCancel ? BRJobState::BR_JOB_STATE_CANCEL_DONE : BRJobState::BR_JOB_STATE_DONE;

        Q_EMIT this->processChanged(tmp_result);
        if (tmp_result.finished_operation_num == tmp_result.sum_operation_num)
        {
            Q_EMIT this->processFinished();
        }

        // 确保前面的信号发送出去后再将状态设置为空闲，这样信号的回调函数能够收到正确的状态值
        if (this->m_state == BRJobState::BR_JOB_STATE_DONE ||
            this->m_state == BRJobState::BR_JOB_STATE_CANCEL_DONE)
        {
            this->m_state = BRJobState::BR_JOB_STATE_IDLE;
            return false;
        }
    }
    return true;
}
}  // namespace BRDaemon
}  // namespace KS
