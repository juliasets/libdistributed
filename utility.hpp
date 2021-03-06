
#if !defined __utility_hpp__
#define __utility_hpp__

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <sstream>
#include <iostream>


namespace Distributed
{

    namespace _utility
    {

        uint64_t rand64 ();
        uint64_t prand64 ();
        std::string uunique_str ();

        struct Address
        {
            std::string hostname;
            unsigned short port;
            bool operator == (const Address & other) const
            {
                if (port != other.port) return false;
                if (hostname != other.hostname) return false;
                return true;
            }
        };

    }

} // Escape both namespaces.

namespace std
{

    template <> struct hash<Distributed::_utility::Address>
    {
        size_t operator () (const Distributed::_utility::Address & a) const
        {
            return hash<string>()(a.hostname + ' ' + to_string(a.port));
        }
    };

}

namespace Distributed
{

    namespace _utility
    {

        struct NodeInfo
        {
            uint64_t id;
            std::unordered_set<Address> addresses;
            std::vector<std::string> services;
            uint64_t last_pinged;
            uint64_t last_success;
            double busyness;
            bool probably_dead () {
                return last_pinged - last_success > 10;
            }
        };

        class Synchronize
        {
            std::recursive_mutex &_mutex;
        public:
            Synchronize (std::recursive_mutex & mutex) : _mutex(mutex)
            {
                _mutex.lock();
            }
            ~Synchronize ()
            {
                _mutex.unlock();
            }
        };

        static class Log {
        public:
            std::stringstream o;
            void flush ()
            {
                std::cerr << o.str();
                std::cerr.flush();
                o.str("");
            }
        } log;

    }

}


std::ostream & operator << (std::ostream & os,
    const Distributed::_utility::NodeInfo & ni);


std::istream& operator >> (std::istream & is,
    Distributed::_utility::NodeInfo & ni);


#endif // !defined __utility_hpp__

