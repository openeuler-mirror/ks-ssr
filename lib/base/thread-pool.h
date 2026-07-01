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

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace KS
{
struct ThreadWorkerInfo
{
    std::thread thread;
    // 线程私有任务，当前线程空闲时优先读取私有任务，然后读取公共任务
    std::queue<std::packaged_task<void()> > tasks;
};

class ThreadPool
{
public:
    ThreadPool(size_t);

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

    template <class F, class... Args>
    auto enqueueByIdx(size_t idx, F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

    ~ThreadPool();

private:
    size_t thread_num_;
    // need to keep track of threads so we can join them
    std::vector<ThreadWorkerInfo> workers_;
    // 公共任务，当线程空闲时会从该任务队列中读取后执行
    std::queue<std::packaged_task<void()> > tasks_;

    // synchronization
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

// add new work item to the pool
template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::packaged_task<return_type()>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task.get_future();
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex_);

        if (this->stop_)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        this->tasks_.emplace(std::move(task));
    }
    this->condition_.notify_one();
    return res;
}

template <class F, class... Args>
auto ThreadPool::enqueueByIdx(size_t idx, F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::packaged_task<return_type()>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task.get_future();
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex_);

        // don't allow enqueueing after stopping the pool
        if (this->stop_)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        /// @warning thread_num_ 为 O 时导致程序收到 SIGFPE 信号 退出
        this->workers_[idx % this->thread_num_].tasks.emplace(std::move(task));
    }
    // 这里正常只需要通知上面插入任务的线程即可，由于condition_.notify_one只能随机通知一个线程，为了简化代码就先全部通知了，反正线程数也不会太多
    this->condition_.notify_all();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex_);
        this->stop_ = true;
    }
    this->condition_.notify_all();
    for (auto& worker : this->workers_)
    {
        worker.thread.join();
    }
}

}  // namespace KS