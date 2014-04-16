
#if !defined __client_hpp__
#define __client_hpp__

#include <mutex>
#include <thread>
#include <string>
#include <boost/asio.hpp>

namespace Distributed {



class ClientJob;



class Client
{

    struct _master
    {
        std::string hostname;
        unsigned short port;
    };

    std::string _key;
    std::vector<_master> masters;

public:

    Client (const std::string & key) : _key(key), masters() {}

    /*
        Add a master.
    */
    void add_master (const std::string & hostname, unsigned short port);

    bool get_slave (std::string & hostname, unsigned short & port) const;

    friend class ClientJob;

};


class ClientJob
{
    std::string _key;

    unsigned short myport;
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    bool success = false;

    std::string result;
    std::thread _waiter;
    std::timed_mutex _wait;

    std::string _hostname;
    unsigned short _port;

    bool initialize (unsigned short port);

    void serve (); // Waits for results, then unlocks _wait and quits.

    void cancel ();

public:

    ClientJob (const ClientJob &) = delete;

    ClientJob (const Client & client) :
        _key(client._key),
        io_service(), acceptor(io_service),
        _waiter(), _wait()
    {
        if (!client.get_slave(_hostname, _port))
            return; // Could not get slave.

        for (unsigned short port = 30000; port < 60000; ++port)
        {
            success = initialize(port);
            if (success) break;
        }

        _wait.lock(); // Have not received result yet.
        _waiter = std::thread(&ClientJob::serve, this); // Wait for result.
    }

    ~ClientJob ()
    {
        if (_waiter.joinable()) _waiter.join();
    }

    /*
        Allow conversion to (void *) to test if opened successfully.
    */
    operator void * () { return (void *) success; }

    /*
        Send the job as a string. Return true on success.
    */
    bool send_job (const std::string & msg);

    /*
        Wait for the result of a job, with a time limit.
    */
    bool get_result (size_t ms, std::string & message);

    /*
        Get the port this ClientJob is waiting on.
    */
    unsigned short port () { return myport; }
};



}

#endif // !defined __client_hpp__

