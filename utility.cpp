
#include "utility.hpp"

#include <random>


using namespace Distributed;
using namespace Distributed::_utility;


uint64_t rand64 ()
{
    static std::random_device rd;
    static std::default_random_engine rng(rd());
    static std::uniform_int_distribution<uint64_t> dist;
    return dist(rd);
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


