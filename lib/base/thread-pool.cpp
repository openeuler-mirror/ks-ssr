/**
 * @file          /kiran-ssr-manager/lib/base/thread-pool.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "lib/base/thread-pool.h"

namespace Kiran
{
ThreadPool::ThreadPool(size_t thread_num) : thread_num_(thread_num),
                                            workers_(thread_num),
                                            stop_(false)
{
    for (size_t i = 0; i < this->thread_num_; ++i)
    {
        workers_[i].thread = std::thread(
            [this, i] {
                while (true)
                {
                    std::packaged_task<void()> task;
                    auto &private_tasks = this->workers_[i].tasks;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex_);
                        this->condition_.wait(lock,
                                              [this, &private_tasks] { return this->stop_ || !this->tasks_.empty() || !private_tasks.empty(); });

                        if (!private_tasks.empty())
                        {
                            task = std::move(private_tasks.front());
                            private_tasks.pop();
                        }
                        else if (!this->tasks_.empty())
                        {
                            task = std::move(this->tasks_.front());
                            this->tasks_.pop();
                        }
                        else if (this->stop_)
                        {
                            return;
                        }
                    }

                    task();
                }
            });
    }
}

}  // namespace Kiran
