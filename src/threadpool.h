/*
  Shellac - A UCI chess engine.
  Copyright (C) 2026 Amber Goulding

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// https://dev.to/ish4n10/making-a-thread-pool-in-c-from-scratch-bnm

#ifndef SHELLAC_THREAD_H
#define SHELLAC_THREAD_H

#include <condition_variable>
#include <functional>
#include <future>
#include <queue>
#include <thread>
#include <vector>

namespace shellac {

class ThreadPool
{
public:
    explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
    ~ThreadPool();

    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

    ThreadPool(ThreadPool&)                  = delete;
    ThreadPool(const ThreadPool&)            = delete;
    ThreadPool& operator=(ThreadPool&&)      = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    void worker();

    std::vector<std::thread>          workers_;
    std::condition_variable           cv_;
    std::mutex                        mutex_;
    std::queue<std::function<void()>> queue_;

    bool stop_{false};
};

template <typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
{
    auto func            = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    auto encapsulatedPtr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

    std::future<std::result_of_t<F(Args...)>> futureObject = encapsulatedPtr->get_future();
    {
        std::unique_lock lock(mutex_);
        queue_.emplace(
            [encapsulatedPtr]
            {
                (*encapsulatedPtr)(); // execute the fx
            });
    }
    cv_.notify_one();
    return futureObject;
}

} // namespace shellac

#endif // SHELLAC_THREAD_H
