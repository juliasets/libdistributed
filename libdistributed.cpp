
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
    JOB = 0,
    PING = 1,
    PUBLISH = 2
};


bool Node::get_random_node (NodeInfo & ni)
{
    SYNCHRONIZED (data_lock)
    {
        if (nodes_by_id.size() < 1) return false;
        auto it = nodes_by_id.begin();
        std::advance(it, prand64() % nodes_by_id.size());
            // TODO: The above search is linear. Is there a better way
            // to do this without making other actions linear?
            // Perhaps a different data structure, or a combination of a few.
        ni = it->second;
        return true;
    }
    return false; // Unreachable.
}


/*
    Find host based on (hostname, port).
    Send:
        PING
        hostname port
    Read:
        remote node's id; if id != 0 and id doesn't match, return false
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
bool Node::ping (std::string hostname, unsigned short port, uint64_t id)
{
    try
    {
        asio::ip::tcp::iostream stream(hostname, std::to_string(port));
        if (!stream) return false;
        stream << PING << std::endl;
        stream << hostname << ' ' << port << std::endl;
        // Wait for information from host.
        uint64_t remote_id;
        stream >> remote_id;
        if (id != 0 && remote_id != id) return false;
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
                if (tempnode.probably_dead())
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
        my id
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
    // Send my id.
    stream << mynodeinfo.id << std::endl;
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


/*
    Publish this node's information by telling a remote host to ping it.
    Send:
        PUBLISH
        my port
*/
bool Node::publish (std::string hostname, unsigned short port)
{
    try
    {
        asio::ip::tcp::iostream stream(hostname, std::to_string(port));
        if (!stream) return false;
        stream << PUBLISH << std::endl;
        stream << myport << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
    return true;
}


/*
    Maintain the network forever.
    Every second:
        Select a node randomly to ping. Continue if no other nodes known.
        For each address of node:
            try to ping address, and if successful, record success
        if never successful, update last_pinged info on node in my records
        if node is probably dead, remove from my records
        TODO: Send my node information somewhere.
*/
void Node::maintain_forever ()
{
    for (;
        // Workaround: try_lock_for() is broken in gcc 4.7, and won't be
        // fixed until at least gcc 4.9
        // See gcc.gnu.org/bugzilla/show_bug.cgi?id=54562
        !maintain_timer.try_lock_until(
            std::chrono::system_clock::now() +
                std::chrono::milliseconds(1000));
        )
    {
        NodeInfo ni;
        std::cout << "maintaining!" << std::endl;
        if (!get_random_node(ni)) continue; // No other nodes known.
        std::cout << "pinging" << std::endl;
        bool succeeded = false;
        for (const Address a : ni.addresses)
        {
            succeeded = ping(a.hostname, a.port, ni.id);
            if (succeeded) break;
        }
        if (!succeeded)
            ni.last_pinged = time(NULL);
        if (ni.probably_dead())
        {
            SYNCHRONIZED (data_lock)
            {
                // Remove node from list of services.
                for (const std::string & service : ni.services)
                    nodes_by_service[service].erase(ni.id);
                nodes_by_id.erase(ni.id);
            }
        }
    }
}


double Node::get_busyness ()
{
    return 0.5; // TODO
}


Job Node::accept ()
{
    try
    {
        for (;;)
        {
            system::error_code error;
            asio::ip::tcp::iostream stream;
            asio::ip::tcp::endpoint remote_ep;
            acceptor.accept(*stream.rdbuf(), remote_ep, error);
            unsigned short port = remote_ep.port();
            std::cout << remote_ep.address().to_string() << ' ' << port << std::endl;
            if (!error)
            {
                unsigned request;
                stream >> request;
                if (request == PING)
                {
                    pong(stream);
                    continue;
                }
                if (request == PUBLISH)
                {
                    unsigned short port;
                    stream >> port;
                    stream.close();
                    ping(remote_ep.address().to_string(), port, 0);
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
        // Send my information to other node (return calls, etc).
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw e;
    }
    return true;
}


