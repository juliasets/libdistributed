
#include "libdistributed.hpp"

#include <random>
#include <chrono>
#include <ctime>
#include <algorithm>

using namespace Distributed::_utility;


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


std::ostream & operator << (std::ostream & os, const NodeInfo & ni)
{
    os << ni.id << std::endl;
    os << ni.addresses.size() << std::endl;
    for (const Address & a : ni.addresses)
        os << a.hostname << ' ' << a.port << std::endl;
    os << ni.services.size() << std::endl;
    for (const std::string & s : ni.services)
        os << s << std::endl;
    os << ni.last_pinged << std::endl;
    os << ni.last_success << std::endl;
    os << ni.busyness << std::endl;
    return os;
}


std::istream& operator >> (std::istream & is, NodeInfo & ni)
{
    is >> ni.id;
    size_t count;
    is >> count;
    for (size_t i = 0; i < count; ++i)
    {
        Address a;
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


enum RequestType
{
    PING = 0,
    JOB = 1
};


uint64_t rand64 ()
{
    static std::random_device rd;
    static std::default_random_engine rng(rd());
    static std::uniform_int_distribution<uint64_t> dist;
    return dist(rd);
}



using namespace Distributed;
using namespace boost;



bool Node::ping (std::string hostname, unsigned short port)
{
    try
    {
        asio::ip::tcp::iostream stream(hostname, std::to_string(port));
        if (!stream) return false;
        stream << PING << std::endl;
        stream << hostname << ' ' << port << std::endl;
        // Wait for information from host.
        size_t count;
        stream >> count;
        NodeInfo tempnode;
        for (size_t i = 0; i < count; ++i)
        {
            stream >> tempnode;
            // Ignore myself.
            if (tempnode.id == mynodeinfo.id) continue;
            // Ignore probably dead nodes.
            if (tempnode.last_pinged - tempnode.last_success > 10) continue;
            auto it = nodes_by_id.find(tempnode.id);
            if (it != nodes_by_id.end())
            { // I have already seen this node.
                if (it->second.last_success > tempnode.last_success)
                    continue; // Ignore old information.
                if (it->second.last_pinged > tempnode.last_pinged)
                    tempnode.last_pinged = it->second.last_pinged;
                // Remove node from list of services.
                for (const std::string & service : it->second.services)
                    nodes_by_service[service].erase(tempnode.id);
                // Merge address sets.
                tempnode.addresses.insert(it->second.addresses.begin(),
                    it->second.addresses.end());
            }
            // Put tempnode in our records.
            nodes_by_id[tempnode.id] = tempnode;
            for (const std::string & service : tempnode.services)
                nodes_by_service[service][tempnode.id] = tempnode;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw e;
    }
    return true;
}


void Node::pong (asio::ip::tcp::iostream & stream)
{
    // First, update information on myself.
    // Insert my address into list of address if it wasn't already there.
    Address a;
    stream >> a.hostname >> a.port;
    mynodeinfo.addresses.insert(a);
    mynodeinfo.last_pinged = mynodeinfo.last_success = time(NULL);
    mynodeinfo.busyness = get_busyness();
    // Next, send everything I know.
    { Synchronize sync(data_lock);
        // Send the total number of nodes I know about, including myself.
        stream << nodes_by_id.size() + 1 << std::endl;
        // Send my own updated information.
        stream << mynodeinfo << std::endl;
        // Send the rest of the nodes I know about.
        for (auto & pair : nodes_by_id)
            stream << pair.second << std::endl;
    }
}


void Node::maintain_forever ()
{
    for (; maintaining;
        maintain_timer.try_lock_for(std::chrono::milliseconds(1000)))
    {
        // TODO
    }
}


double Node::get_busyness ()
{
    return 0.5;
}


Job Node::accept ()
{
    try
    {
        for (;;)
        {
            system::error_code error;
            asio::ip::tcp::iostream stream;
            acceptor.accept(*stream.rdbuf(), error);
            if (!error)
            {
                unsigned request;
                stream >> request;
                if (request == PING)
                {
                    pong(stream);
                    continue;
                }
                Job job;
                stream >> job.service;
                size_t length;
                stream >> length;
                job.message.resize(length);
                stream.read(&job.message[0], length);
                stream << "accepted" << std::endl;
                return std::move(job);
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw e;
    }
}


bool Node::send (Job job)
{
    try
    {
        // TODO: ?
        // Send that we are sending a job.
        // Send name of service.
        // Send length of message.
        // Send message.
        // Listen for "accepted".
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw e;
    }
    return true;
}


