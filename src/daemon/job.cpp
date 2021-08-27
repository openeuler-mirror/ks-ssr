/**
 * @file          /kiran-ssr-manager/src/daemon/job.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/job.h"
#include "src/daemon/plugins.h"

namespace Kiran
{
namespace Daemon
{
int64_t Job::job_count_ = 0;

std::shared_ptr<Job> Job::create()
{
    std::shared_ptr<Job> job(new Job(++Job::job_count_));
    return job;
}

Job::Job(int64_t job_id) : job_id_(job_id),
                           state_(SSRJobState::SSR_JOB_STATE_IDLE),
                           need_cancel_(false)
{
}

Job::~Job()
{
    if (this->idle_handler_)
    {
        this->idle_handler_.disconnect();
    }
}

std::shared_ptr<Operation> Job::add_operation(const std::string &plugin_name,
                                              const std::string &reinforcement_name,
                                              std::function<std::string(void)> func)
{
    auto operation = std::make_shared<Operation>();

    operation->job_id = this->job_id_;
    operation->operation_id = this->operations_.size() + 1;
    operation->plugin_name = plugin_name;
    operation->reinforcement_name = reinforcement_name;
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

bool Job::run_sync()
{
    KLOG_DEBUG("job id: %d.", this->job_id_);

    RETURN_VAL_IF_FALSE(this->state_ == SSRJobState::SSR_JOB_STATE_IDLE, false);
    this->state_ = SSRJobState::SSR_JOB_STATE_RUNNING;

    this->run_init();

    for (auto iter : this->operations_)
    {
        OperationResult result;
        auto operation = iter.second;
        result.operation_id = operation->operation_id;
        result.result = operation->func();
        // operation->func_result = operation->func();
        ++this->job_result_.finished_operation_num;
        // this->job_result_.finished_operations.push_back(operation->operation_id);
        this->job_result_.current_finished_operations.push_back(std::move(result));

        if (this->job_result_.finished_operation_num == this->job_result_.sum_operation_num)
        {
            this->state_ = SSRJobState::SSR_JOB_STATE_DONE;
        }
        this->process_changed_.emit(this->job_result_);
    }
    this->state_ = SSRJobState::SSR_JOB_STATE_IDLE;
    return true;
}

bool Job::run_async()
{
    KLOG_DEBUG("job id: %d.", this->job_id_);

    RETURN_VAL_IF_FALSE(this->state_ == SSRJobState::SSR_JOB_STATE_IDLE, false);
    this->state_ = SSRJobState::SSR_JOB_STATE_RUNNING;

    this->run_init();

    // 空闲时监听任务完成的情况
    auto idle = Glib::MainContext::get_default()->signal_idle();
    this->idle_handler_ = idle.connect(sigc::mem_fun(this, &Job::idle_check_operation));

    auto &thread_pool = Plugins::get_instance()->get_thread_pool();
    {
        std::lock_guard<std::mutex> guard(this->operations_mutex_);
        for (auto iter : this->operations_)
        {
            thread_pool.enqueue(std::bind(&Job::run_operation, this, iter.second));
        }
    }
    return true;
}

bool Job::cancel()
{
    // 只有在运行中的任务才可以取消
    RETURN_VAL_IF_FALSE(this->state_ == SSRJobState::SSR_JOB_STATE_RUNNING, false);
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

void Job::run_operation(std::shared_ptr<Operation> operation)
{
    RETURN_IF_FALSE(operation);

    OperationResult result;
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

bool Job::idle_check_operation()
{
    JobResult tmp_result;
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
        KLOG_DEBUG("sum num: %d, finished num: %d, running num: %d, current finished num: %d.",
                   tmp_result.sum_operation_num,
                   tmp_result.finished_operation_num,
                   tmp_result.running_operations.size(),
                   tmp_result.current_finished_operations.size());

        if (tmp_result.finished_operation_num == tmp_result.sum_operation_num)
        {
            this->state_ = this->need_cancel_ ? SSRJobState::SSR_JOB_STATE_CANCEL_DONE : SSRJobState::SSR_JOB_STATE_DONE;
        }

        this->process_changed_.emit(tmp_result);

        // 确保前面的信号发送出去后再将状态设置为空闲，这样信号的回调函数能够收到正确的状态值
        if (this->state_ == SSRJobState::SSR_JOB_STATE_DONE ||
            this->state_ == SSRJobState::SSR_JOB_STATE_CANCEL_DONE)
        {
            this->state_ = SSRJobState::SSR_JOB_STATE_IDLE;
            return false;
        }
    }
    return true;
}
}  // namespace Daemon
}  // namespace Kiran
