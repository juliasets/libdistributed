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
    
public:
    
    std::function<void ()> pop();
    
    ThreadPool(int numThreads);
    
    bool isRunning() { return running || (!tasks.empty()); };
    
    void shutdown();
    
    void execute(std::function<void ()> func);
};
#endif //_THREADPOOL_HPP
