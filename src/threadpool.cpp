#include "threadpool.h"

ThreadPool::ThreadPool(unsigned int num_threads /* = std::thread::hardware_concurrency() */): 
isStopped(false), threads(), tasks(), tasks_mutex(), tasks_cv() {
    if (0 == num_threads) {
        num_threads = 1;
    }
    
    for (size_t i = 0; i < num_threads; ++i) {        

        // contruct worker thread for each system thread
        threads.emplace_back([this](){
            std::function<void()> task;
            while (true) {
                {
                    std::unique_lock<std::mutex> lock(tasks_mutex);

                    tasks_cv.wait(lock, [this](){
                        return !tasks.empty() || isStopped;
                    });

                    if (isStopped && tasks.empty()) {
                        return;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(tasks_mutex);
        isStopped = true;
    }

    tasks_cv.notify_all();
    for (auto& thread : threads) {
        thread.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(tasks_mutex);
        tasks.emplace(std::move(task));
    }

    tasks_cv.notify_one();
}