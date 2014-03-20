
#include "libdistributed.hpp"
#include "utility_macros.hpp"

#include <random>
#include <chrono>
#include <ctime>
#include <algorithm>

using namespace Distributed;
using namespace Distributed::_utility;
using namespace boost;


enum RequestType
{
    PING = 0,
    JOB = 1
};


/*
    Find host based on (hostname, port).
    Send:
        PING
        hostname port
    Read:
        number of nodes
        for each node:
            read node
            ignore if myself, ignore if dead
            if I've seen this, ignore old information
            if I've seen this, remove node from records
            if I've seen this, merge address sets
            add node to records
    Returns: true if remote host responded as expected, false otherwise
*/
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
            SYNCHRONIZED (data_lock)
            {
                // Ignore myself.
                if (tempnode.id == mynodeinfo.id) continue;
                // Ignore probably dead nodes.
                if (tempnode.last_pinged - tempnode.last_success > 10)
                    continue;
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
            } // End Synchronization
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
    return true;
}


/*
    Respond to ping. PING has already been received.
    Read:
        hostname port
        insert hostname and port into list of addresses
        update my own node information before sending it
    Write:
        number of total nodes (including myself)
        send my node
        for each other node I know about:
            send node
*/
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
    SYNCHRONIZED (data_lock)
    {
        // Send the total number of nodes I know about, including myself.
        stream << nodes_by_id.size() + 1 << std::endl;
        // Send my own updated information.
        stream << mynodeinfo << std::endl;
        // Send the rest of the nodes I know about.
        for (auto & pair : nodes_by_id)
            stream << pair.second << std::endl;
    } // End synchronization
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


