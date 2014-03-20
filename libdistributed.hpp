
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

    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;

    _utility::NodeInfo mynodeinfo;
    std::map<uint64_t, _utility::NodeInfo> nodes_by_id;
    std::unordered_map<std::string,
        std::unordered_map<uint64_t, _utility::NodeInfo>> nodes_by_service;

    std::recursive_mutex data_lock;

    bool get_random_node (_utility::NodeInfo & ni);

    std::thread maintainer;
    bool maintaining = true;
    std::timed_mutex maintain_timer;
    void maintain_forever ();

    bool ping (std::string hostname, unsigned short port);

    void pong (boost::asio::ip::tcp::iostream & stream);

    double get_busyness ();

public:

    Node (unsigned short port) :
        io_service(),
        acceptor(io_service,
            boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        maintainer()
    {
        mynodeinfo.id = _utility::rand64();
        mynodeinfo.last_pinged = 0;
        mynodeinfo.last_success = 0;
        mynodeinfo.busyness = get_busyness();

        maintainer = std::thread(&Node::maintain_forever, this);
            // Start maintainer thread.
    }

    ~Node ()
    {
        maintaining = false;
        maintain_timer.unlock(); // Cancel maintain timer early.
        maintainer.join();
    }

    bool join_network (std::string hostname, unsigned short port)
    {
        return ping(hostname, port);
    }

    Job accept ();
    bool send (Job job);

    void publish (std::string service);

};


}

#endif // !defined __libdistributed_hpp__

