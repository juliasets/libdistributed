#ifndef _THREADPOOL_HPP
#define _THREADPOOL_HPP

#include <thread>
#include <mutex>
#include <functional>
#include <queue>
#include <vector>
#include <condition_variable>

namespace Distributed
{


class ThreadPool
{
private:
    std::queue<std::function<void ()>> tasks;
    std::mutex queueLock;
    std::condition_variable cv;
    std::vector<std::thread> threads;
    bool running;
    
    bool isRunning () { return running || (!tasks.empty()); };
    
    void performTask ()
    {
        while (isRunning())
        {
            std::function<void ()> task = pop();
            if (task == NULL)
                continue;
            task();
        }
    }
    
    std::function<void ()> pop ()
    {
        std::unique_lock<std::mutex> ul(queueLock);
        cv.wait(ul, [this](){ return (!tasks.empty())||(!running);});
        if (tasks.empty())
            return NULL;
        std::function<void ()> task = tasks.front();
        tasks.pop();
        return task;
    }
    
public:
    
    /*
    Create ThreadPool. Starts running numThreads threads with the function
    performTask.
    */
    ThreadPool (int numThreads)
    {
        running = true;
        tasks = std::queue<std::function<void ()>>();
        for (int i = 0; i < numThreads; i++)
        {
            std::thread th (&ThreadPool::performTask, this);
            threads.push_back(std::move(th));
        }
    }
    
    /*
    Refuse new tasks, finish off all tasks in the queue, and join the worker 
    threads.
    */
    void shutdown ()
    {
        running = false;
        cv.notify_all();
        for (std::thread &t : threads)
        {
            t.join();
        }
    }
    
    /*
    Add new task to thread pool. The function is pushed to the end of the queue.
    */
    void execute (std::function<void ()> func)
    {
        if (!running) //if shutdown has been called, refuse new tasks
            return;
        if (running)
        {
            queueLock.lock();
            tasks.push(func);
            queueLock.unlock();
            cv.notify_all();
        }
    }
};


} // End of namespace Distributed

#endif //_THREADPOOL_HPP
