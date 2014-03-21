#ifndef _THREADPOOL_HPP
#define _THREADPOOL_HPP

#include <thread>
#include <mutex>
#include <functional>
#include <queue>
#include <vector>
#include <condition_variable>

class ThreadPool
{
private:
    std::queue<std::function<void ()>> tasks;
    std::mutex queueLock;
    std::condition_variable cv;
    std::vector<std::thread> threads;
    bool running;
    
    void performTask()
    {
        while (isRunning())
        {
            std::function<void ()> task = pop();
            if (task == NULL)
                continue;
            task();
        }
    };
    
public:
    
    std::function<void ()> pop()
    {
        std::unique_lock<std::mutex> ul(queueLock);
        cv.wait(ul, [this](){ return (!tasks.empty())||(!running);});
        if (tasks.empty())
            return NULL;
        std::function<void ()> task = tasks.front();
        tasks.pop();
        return task;
    };
    
    ThreadPool(int numThreads)
    {
        running = true;
        tasks = std::queue<std::function<void ()>>();
        for (int i = 0; i < numThreads; i++)
        {
            std::thread th (&ThreadPool::performTask, this);
            threads.push_back(std::move(th));
        }
    };
    
    bool isRunning() { return running || (!tasks.empty()); };
    
    void shutdown()
    {
        running = false;
        cv.notify_all();
        for (std::thread &t : threads)
        {
            t.join();
        }
    };
    
    void execute(std::function<void ()> func)
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
};
#endif //_THREADPOOL_HPP
