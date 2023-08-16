/**
 * @file          /kiran-sse-manager/src/daemon/sse-job.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <mutex>
#include "lib/base/base.h"

namespace Kiran
{
struct SSEOperationResult
{
    // 操作ID
    int32_t operation_id;
    // 操作结果
    std::string result;
};
using SSEOperationResultVec = std::vector<std::shared_ptr<SSEOperationResult>>;

struct SSEOperation
{
    // 任务ID
    int64_t job_id;
    // 操作ID
    int32_t operation_id;
    // 插件名
    std::string plugin_name;
    // 加固项名
    std::string reinforcement_name;
    // 执行的函数
    std::function<std::string(void)> func;
};
using SSEOperationVec = std::vector<std::shared_ptr<SSEOperation>>;

// 任务返回的结果
struct SSEJobResult
{
    SSEJobResult() : job_id(0),
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
    std::vector<int32_t> running_operations;
    // 从上一次发送信号到这一次完成的操作
    // std::vector<int32_t> current_finished_operations;
    std::vector<SSEOperationResult> current_finished_operations;
};

class SSEJob
{
public:
    virtual ~SSEJob();

    // 创建任务
    static std::shared_ptr<SSEJob> create();

    // 获取任务ID
    int64_t get_id() { return this->job_id_; };

    // 获取任务状态
    SSEJobState get_state() { return this->state_; };

    // 获取操作
    std::shared_ptr<SSEOperation> get_operation(int32_t operation_id) { return MapHelper::get_value(this->operations_, operation_id); };

    // 添加一个操作，返回操作ID
    std::shared_ptr<SSEOperation> add_operation(const std::string &plugin_name,
                                                const std::string &reinforcement_name,
                                                std::function<std::string(void)> func);

    // 同步运行任务
    bool run_sync();

    // 异步运行任务
    bool run_async();

    // 取消任务，只对run_async有效
    bool cancel();

    // 任务进度变化的信号
    sigc::signal<void, const SSEJobResult &> &signal_process_changed() { return this->process_changed_; };

private:
    SSEJob(int64_t job_id);

    // 运行前初始化
    void run_init();

    // 线程中运行操作
    void run_operation(std::shared_ptr<SSEOperation> operation);

    // 空闲时监听任务的进度
    bool idle_check_operation();

private:
    int64_t job_id_;
    SSEJobState state_;
    sigc::connection idle_handler_;
    // 操作集合
    std::map<int32_t, std::shared_ptr<SSEOperation>> operations_;
    // 任务执行的结果
    SSEJobResult job_result_;
    // 操作互斥
    std::mutex operations_mutex_;
    // 任务需要取消
    bool need_cancel_;

    static int64_t job_count_;

    sigc::signal<void, const SSEJobResult &> process_changed_;
};

using SSEJobVec = std::vector<std::shared_ptr<SSEJob>>;
}  // namespace Kiran
