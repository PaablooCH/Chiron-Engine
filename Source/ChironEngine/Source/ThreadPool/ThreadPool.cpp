#include "Pch.h"
#include "ThreadPool.h"

ThreadPool::ThreadPool(UINT threadNumber) : _activeTasks(0), _stop(false)
{
    if (threadNumber == 0)
    {
        LOG_WARNING("Thread number is 0, setting to 1");
        threadNumber = 1;
    }
    if (threadNumber > std::thread::hardware_concurrency())
    {
        threadNumber = std::thread::hardware_concurrency();
    }
    for (UINT i = 0; i < threadNumber; i++)
    {
        _workers.emplace_back([this] 
            {
                while (true)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(_tasksMutex);

                        _cv.wait(lock, [this] 
                            {
                                return !_tasks.empty() || _stop;
                            }
                        );

                        if (_stop && _tasks.empty()) 
                        {
                            return;
                        }

                        task = std::move(_tasks.front());
                        _tasks.pop();
                        _activeTasks++;
                    }

                    task();

                    {
                        std::unique_lock<std::mutex> lock(_tasksMutex);
                        _activeTasks--;
                        if (_tasks.empty() && _activeTasks == 0) 
                        {
                            _cv.notify_all();
                        }
                    }
                }
            }
        );
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(_tasksMutex);
        _stop = true;
    }

    _cv.notify_all();

    for (auto& worker : _workers) 
    {
        worker.join();
    }
    _workers.clear();
}

void ThreadPool::AddTask(const std::function<void()>& task)
{
    {
        std::unique_lock<std::mutex> lock(_tasksMutex);
        _tasks.push(task);
    }
    _cv.notify_one();
}

void ThreadPool::WaitForCompletion()
{
    std::unique_lock<std::mutex> lock(_tasksMutex);
    _cv.wait(lock, [this]
        {
            return _tasks.empty() && _activeTasks == 0;
        }
    );
}