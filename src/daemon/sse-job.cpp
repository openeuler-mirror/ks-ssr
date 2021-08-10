/**
 * @file          /kiran-sse-manager/lib/core/sse-job.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/sse-job.h"
#include "src/daemon/sse-plugins.h"

namespace Kiran
{
int64_t SSEJob::job_count_ = 0;

std::shared_ptr<SSEJob> SSEJob::create()
{
    std::shared_ptr<SSEJob> job(new SSEJob(++SSEJob::job_count_));
    return job;
}

SSEJob::SSEJob(int64_t job_id) : job_id_(job_id),
                                 state_(SSEJobState::SSE_JOB_STATE_IDLE),
                                 need_cancel_(false)
{
}

SSEJob::~SSEJob()
{
    if (this->idle_handler_)
    {
        this->idle_handler_.disconnect();
    }
}

std::shared_ptr<SSEOperation> SSEJob::add_operation(const std::string &plugin_name,
                                                    const std::string &reinforcement_name,
                                                    std::function<std::string(void)> func)
{
    auto operation = std::make_shared<SSEOperation>();

    operation->job_id = this->job_id_;
    operation->operation_id = this->operations_.size() + 1;
    operation->plugin_name = plugin_name;
    operation->reforcement_name = reinforcement_name;
    operation->func = std::move(func);

    auto iter = this->operations_.emplace(operation->operation_id, operation);
    if (!iter.second)
    {
        // 正常不应该执行到这里
        KLOG_WARNING("The operation %d is already exist.", operation->operation_id);
        return nullptr;
    }
    return operation;
}

bool SSEJob::run_sync()
{
    KLOG_DEBUG("job id: %d.", this->job_id_);

    RETURN_VAL_IF_FALSE(this->state_ == SSEJobState::SSE_JOB_STATE_IDLE, false);
    this->state_ = SSEJobState::SSE_JOB_STATE_RUNNING;

    this->run_init();

    for (auto iter : this->operations_)
    {
        SSEOperationResult result;
        auto operation = iter.second;
        result.operation_id = operation->operation_id;
        result.result = operation->func();
        // operation->func_result = operation->func();
        ++this->job_result_.finished_operation_num;
        // this->job_result_.finished_operations.push_back(operation->operation_id);
        this->job_result_.current_finished_operations.push_back(std::move(result));

        if (this->job_result_.finished_operation_num == this->job_result_.sum_operation_num)
        {
            this->state_ = SSEJobState::SSE_JOB_STATE_DONE;
        }
        this->process_changed_.emit(this->job_result_);
    }
    this->state_ = SSEJobState::SSE_JOB_STATE_IDLE;
    return true;
}

bool SSEJob::run_async()
{
    KLOG_DEBUG("job id: %d.", this->job_id_);

    RETURN_VAL_IF_FALSE(this->state_ == SSEJobState::SSE_JOB_STATE_IDLE, false);
    this->state_ = SSEJobState::SSE_JOB_STATE_RUNNING;

    this->run_init();

    // 空闲时监听任务完成的情况
    auto idle = Glib::MainContext::get_default()->signal_idle();
    this->idle_handler_ = idle.connect(sigc::mem_fun(this, &SSEJob::idle_check_operation));

    auto &thread_pool = SSEPlugins::get_instance()->get_thread_pool();
    {
        std::lock_guard<std::mutex> guard(this->operations_mutex_);
        for (auto iter : this->operations_)
        {
            thread_pool.enqueue(std::bind(&SSEJob::run_operation, this, iter.second));
        }
    }
    return true;
}

bool SSEJob::cancel()
{
    // 只有在运行中的任务才可以取消
    RETURN_VAL_IF_FALSE(this->state_ == SSEJobState::SSE_JOB_STATE_RUNNING, false);
    this->need_cancel_ = true;
    return true;
}

void SSEJob::run_init()
{
    this->job_result_.job_id = this->job_id_;
    this->job_result_.sum_operation_num = this->operations_.size();
    this->job_result_.finished_operation_num = 0;
    this->job_result_.running_operations.clear();
    this->job_result_.current_finished_operations.clear();
    this->need_cancel_ = false;
}

void SSEJob::run_operation(std::shared_ptr<SSEOperation> operation)
{
    RETURN_IF_FALSE(operation);

    SSEOperationResult result;
    result.operation_id = operation->operation_id;

    KLOG_DEBUG("running operation: %d, job id: %d.", operation->operation_id, operation->job_id);

    {
        std::lock_guard<std::mutex> guard(this->operations_mutex_);
        this->job_result_.running_operations.push_back(operation->operation_id);
        this->job_result_.queue_is_changed = true;
    }

    // 任务如果已经取消，则不再执行插件函数
    if (!this->need_cancel_)
    {
        result.result = operation->func();
    }

    KLOG_DEBUG("finished operation: %d, job id: %d.", operation->operation_id, operation->job_id);

    {
        std::lock_guard<std::mutex> guard(this->operations_mutex_);
        ++this->job_result_.finished_operation_num;
        // 从正在运行的队列中删除
        auto iter = std::remove(this->job_result_.running_operations.begin(), this->job_result_.running_operations.end(), operation->operation_id);
        this->job_result_.running_operations.erase(iter, this->job_result_.running_operations.end());
        // 添加到已完成队列
        this->job_result_.current_finished_operations.push_back(std::move(result));
        this->job_result_.queue_is_changed = true;
    }
}

bool SSEJob::idle_check_operation()
{
    SSEJobResult tmp_result;
    {
        std::lock_guard<std::mutex> guard(this->operations_mutex_);
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
        KLOG_DEBUG("sum finished num: %d, finished num: %d, running num: %d, current finished num: %d.",
                   tmp_result.finished_operation_num,
                   tmp_result.sum_operation_num,
                   tmp_result.running_operations.size(),
                   tmp_result.current_finished_operations.size());

        if (tmp_result.finished_operation_num == tmp_result.sum_operation_num)
        {
            this->state_ = this->need_cancel_ ? SSEJobState::SSE_JOB_STATE_CANCEL_DONE : SSEJobState::SSE_JOB_STATE_DONE;
        }

        this->process_changed_.emit(tmp_result);

        // 确保前面的信号发送出去后再将状态设置为空闲，这样信号的回调函数能够收到正确的状态值
        if (this->state_ == SSEJobState::SSE_JOB_STATE_DONE ||
            this->state_ == SSEJobState::SSE_JOB_STATE_CANCEL_DONE)
        {
            this->state_ = SSEJobState::SSE_JOB_STATE_IDLE;
            return false;
        }
    }
    return true;
}
}  // namespace Kiran
