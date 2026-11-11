/*
 * Copyright (C) 2022 The V-Gears Team
 *
 * This file is part of V-Gears
 *
 * V-Gears is free software: you can redistribute it and/or modify it under
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.0 (GPLv3) of the License.
 *
 * V-Gears is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#pragma once

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

/**
 * A fixed number of threads that run tasks.
 *
 * Meant for independent jobs that don't touch shared state, like converting one file with an external program.
 * Tasks must not throw: an exception that escapes a task is swallowed.
 */
class TaskPool{

    public:

        /**
         * Constructor.
         *
         * @param[in] threads Number of threads. Zero uses the number of cores, at most 8 (more doesn't help when
         * the tasks write to the same disk).
         */
        explicit TaskPool(unsigned threads = 0){
            if (threads == 0) threads = std::min(std::max(std::thread::hardware_concurrency(), 1u), 8u);
            for (unsigned i = 0; i < threads; ++ i) workers_.emplace_back([this](){Work();});
        }

        /**
         * Destructor. Waits for the pending tasks.
         */
        ~TaskPool(){
            {
                std::unique_lock<std::mutex> lock(mutex_);
                stop_ = true;
            }
            work_available_.notify_all();
            for (std::thread& worker : workers_) worker.join();
        }

        TaskPool(const TaskPool&) = delete;
        TaskPool& operator=(const TaskPool&) = delete;

        /**
         * Adds a task. It starts as soon as a thread is free.
         */
        void Run(std::function<void()> task){
            {
                std::unique_lock<std::mutex> lock(mutex_);
                queue_.push_back(std::move(task));
                ++ pending_;
            }
            work_available_.notify_one();
        }

        /**
         * Blocks until every task added so far has finished.
         */
        void Wait(){
            std::unique_lock<std::mutex> lock(mutex_);
            all_done_.wait(lock, [this](){return pending_ == 0;});
        }

        /**
         * Gets the number of threads.
         */
        unsigned Threads() const{return static_cast<unsigned>(workers_.size());}

    private:

        void Work(){
            while (true){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    work_available_.wait(lock, [this](){return stop_ || !queue_.empty();});
                    // On stop, the remaining tasks still run.
                    if (queue_.empty()) return;
                    task = std::move(queue_.front());
                    queue_.pop_front();
                }
                try{task();}
                catch (...){}
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    -- pending_;
                }
                all_done_.notify_all();
            }
        }

        std::mutex mutex_;
        std::condition_variable work_available_;
        std::condition_variable all_done_;
        std::deque<std::function<void()>> queue_;
        std::vector<std::thread> workers_;
        size_t pending_ = 0;
        bool stop_ = false;
};
