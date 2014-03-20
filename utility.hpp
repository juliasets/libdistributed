
#if !defined __utility_hpp__
#define __utility_hpp__

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <iostream>
#include <random>


namespace Distributed
{

    namespace _utility
    {

        uint64_t rand64 ()
        {
            static std::random_device rd;
            static std::default_random_engine rng(rd());
            static std::uniform_int_distribution<uint64_t> dist;
            return dist(rd);
        }

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

    }

}


std::ostream & operator << (std::ostream & os,
    const Distributed::_utility::NodeInfo & ni)
{
    os << ni.id << std::endl;
    os << ni.addresses.size() << std::endl;
    for (const auto & a : ni.addresses)
        os << a.hostname << ' ' << a.port << std::endl;
    os << ni.services.size() << std::endl;
    for (const std::string & s : ni.services)
        os << s << std::endl;
    os << ni.last_pinged << std::endl;
    os << ni.last_success << std::endl;
    os << ni.busyness << std::endl;
    return os;
}


std::istream& operator >> (std::istream & is,
    Distributed::_utility::NodeInfo & ni)
{
    is >> ni.id;
    size_t count;
    is >> count;
    for (size_t i = 0; i < count; ++i)
    {
        Distributed::_utility::Address a;
        is >> a.hostname >> a.port;
        ni.addresses.insert(a);
    }
    is >> count;
    ni.services.resize(count);
    for (size_t i = 0; i < count; ++i)
        is >> ni.services[i];
    is >> ni.last_pinged;
    is >> ni.last_success;
    is >> ni.busyness;
    return is;
}


#endif // !defined __utility_hpp__

