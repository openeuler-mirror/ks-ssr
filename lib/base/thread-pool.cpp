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

#include "lib/base/thread-pool.h"

namespace KS
{
ThreadPool::ThreadPool(size_t thread_num) : thread_num_(thread_num),
                                            workers_(thread_num),
                                            stop_(false)
{
    for (size_t i = 0; i < this->thread_num_; ++i)
    {
        this->workers_[i].thread = std::thread(
            [this, i]
            {
                while (true)
                {
                    std::packaged_task<void()> task;
                    auto &private_tasks = this->workers_[i].tasks;
                    // static std::chrono::microseconds du(500);

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex_);
                        this->condition_.wait(lock,
                                              [this, &private_tasks]
                                              { return this->stop_ || !this->tasks_.empty() || !private_tasks.empty(); });

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

                    if (task.valid())
                    {
                        task();
                    }
                }
            });
    }
}

}  // namespace KS
