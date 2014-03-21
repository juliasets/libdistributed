
#if !defined __libdistributed_hpp__
#define __libdistributed_hpp__

#include "utility.hpp"

#include <boost/asio.hpp>

#include <cstdint>
#include <iostream>
#include <exception>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace Distributed {


struct Job
{
    std::string service;
    std::string message;
};


class Node
{

    unsigned short myport;
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;

    _utility::NodeInfo mynodeinfo;
    std::map<uint64_t, _utility::NodeInfo> nodes_by_id;
    std::unordered_map<std::string,
        std::unordered_map<uint64_t, _utility::NodeInfo>> nodes_by_service;

    std::recursive_mutex data_lock;

    bool get_random_node (_utility::NodeInfo & ni);
    bool get_random_node (_utility::NodeInfo & ni,
        const std::string & service);

    std::thread maintainer;
    std::timed_mutex maintain_timer;
    void maintain_forever ();

    bool ping (const std::string & hostname, unsigned short port,
        uint64_t id);
    void pong (boost::asio::ip::tcp::iostream & stream);
    bool publish (const std::string & hostname, unsigned short port);
    bool test (const std::string & hostname, unsigned short port,
        uint64_t id);

    double get_busyness ();

public:

    Node (unsigned short port) :
        myport(port),
        io_service(),
        acceptor(io_service,
            boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        mynodeinfo(),
        maintainer(),
        maintain_timer()
    {
        mynodeinfo.id = _utility::rand64();
        if (mynodeinfo.id == 0) mynodeinfo.id = 1; // Must be non-zero.
        mynodeinfo.last_pinged = 0;
        mynodeinfo.last_success = 0;
        mynodeinfo.busyness = get_busyness();

        maintain_timer.lock(); // Initialize timer.
        maintainer = std::thread(&Node::maintain_forever, this);
            // Start maintainer thread.
    }

    ~Node ()
    {
        maintain_timer.unlock(); // Cancel maintain timer early.
        maintainer.join();
    }

    bool join_network (std::string hostname, unsigned short port)
    {
        return ping(hostname, port, 0);
    }

    Job accept ();
    bool send (Job job);

    void provide_service (const std::string & service);
    void rescind_service (const std::string & service);

};


}

#endif // !defined __libdistributed_hpp__

