#pragma once
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
    ThreadPool(unsigned int = std::thread::hardware_concurrency());
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;

    void enqueue(std::function<void()>);

private:
    bool isStopped;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mutex;
    std::condition_variable tasks_cv;
};
