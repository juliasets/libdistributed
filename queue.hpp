
#if !defined __queue_hpp__
#define __queue_hpp__

#include "utility.hpp"

#include <mutex>
#include <queue>
#include <utility>

namespace Distributed {
namespace _utility {



template <class T>
class Queue
{

    std::recursive_mutex lock;
    std::queue<T> q;

public:

    Queue () {}

    void push (T obj)
    {
        SYNCHRONIZED (lock) q.push();
    }

    T pop ()
    {
        SYNCHRONIZED (lock)
        {
            T ret = std::move(q.front());
            q.pop();
            return std::move(ret);
        }
    }

};



}
}

#endif // !defined __queue_hpp__

