/**
 * @file          /ks-ssr-manager/lib/base/thread-pool.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#if (__GNUC__ < 5) || (__GNUC__ == 4 && __GNUC_MINOR__ <= 9)

#include <glibmm.h>

namespace KS
{
class ThreadPool
{
public:
    ThreadPool(size_t);

    void enqueue(const sigc::slot<void>& slot);
    // 不支持按照idx分配线程，这里直接调用enqueue函数随机分配
    void enqueue_by_idx(size_t idx, const sigc::slot<void>& slot);

    virtual ~ThreadPool(){};

private:
    Glib::ThreadPool thread_pool_;
};
}  // namespace KS

#else

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
    auto enqueue_by_idx(size_t idx, F&& f, Args&&... args)
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
auto ThreadPool::enqueue_by_idx(size_t idx, F&& f, Args&&... args)
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

#endif