
#if !defined __utility_macros_hpp__
#define __utility_macros_hpp__

#include <utility>


#define SYNCHRONIZED(lock) for (std::pair<Synchronize, bool> \
    __(lock, true); __.second; __.second = false)


#endif // !defined __utility_macros_hpp__

