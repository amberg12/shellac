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

//
// Created by amber on 07/03/2026.
//

#include "threadpool.h"

namespace shellac {
ThreadPool::ThreadPool(const size_t threadCount)
{
    for (size_t i = 0; i < threadCount; ++i) {
        workers_.emplace_back(&ThreadPool::worker, this);
    }
}

void ThreadPool::worker() {
    while (true) {
        std::function<void()> currentTask;
        {
            std::unique_lock lock(mutex_);
            cv_.wait(lock, [this]() {
                return stop_ || !queue_.empty();
            });

            if (stop_ && queue_.empty())
                break;
            if (queue_.empty())
                continue;

            currentTask = queue_.front();
            queue_.pop();
        }
        currentTask();
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }

    cv_.notify_all();
    for (auto& worker : workers_) {
        worker.join();
    }
}

} // namespace shellac
