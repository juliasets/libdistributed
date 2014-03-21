#include "ThreadPool.hpp"

std::function<void ()> ThreadPool::pop()
{
    std::unique_lock<std::mutex> ul(queueLock);
    cv.wait(ul, [this](){ return !tasks.empty();});
    std::function<void ()> task = tasks.front();
    tasks.pop();
    return task;
};

void ThreadPool::shutdown()
{
    running = false;
    for (std::thread &t : threads)
    {
        t.join();
    }
};

void ThreadPool::execute(std::function<void ()> func)
{
    if (func == NULL)
        return;
    if (running)
    {
        queueLock.lock();
        tasks.push(func);
        queueLock.unlock();
        cv.notify_all();
    }
};

void performTask(ThreadPool * tp)
{
    while (tp->isRunning())
    {
        std::function<void ()> task = tp->pop();
        task();
    }
};


ThreadPool::ThreadPool(int numThreads)
{
    running = true;
    tasks = std::queue<std::function<void ()>>();
    for (int i = 0; i < numThreads; i++)
    {
        std::thread th (performTask, this);
        threads.push_back(std::move(th));
    }
};


