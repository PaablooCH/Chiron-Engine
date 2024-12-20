#pragma once

class ThreadPool
{
public:
    ThreadPool(UINT threadNumber = std::thread::hardware_concurrency());
    ~ThreadPool();

    void AddTask(const std::function<void()>& task);

private:
    std::vector<std::thread> _workers;

    std::condition_variable _cv;

    std::queue<std::function<void()>> _tasks;
    std::mutex _tasksMutex;

    bool _stop;
};
