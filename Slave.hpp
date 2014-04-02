
#if !defined __slave_hpp__
#define __slave_hpp__

#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <boost/asio.hpp>

namespace Distributed {



class SlaveJob
{
    std::string _hostname;
    unsigned short _port;
    std::string _message;

public:

    SlaveJob () {}

    SlaveJob (const std::string & hostname, unsigned short port,
        const std::string & message) :
        _hostname(hostname), _port(port), _message(message) {}

    /*
        Get the job that was sent to this slave as a string.
    */
    std::string get_job () { return _message; }

    /*
        Send the result of the computation back to the client.
    */
    void send_result (const std::string & msg)
    {
        boost::asio::ip::tcp::iostream
            stream(_hostname, std::to_string(_port));
        if (!stream) return;
        stream << msg;
    }
};


class Slave
{

    static unsigned short lowport;
    unsigned short myport;
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    bool success = false;

    double load ();

    bool initialize (unsigned short port);

    struct _master
    {
        std::string hostname;
        unsigned short port;
    };

    std::recursive_mutex master_lock;
    std::vector<_master> masters;

    std::thread maintainer;
    std::timed_mutex maintain_timer;
    void maintain_forever ();

public:

    Slave (const Slave &) = delete;

    Slave (unsigned short port = 0) :
        io_service(),
        acceptor(io_service),
        maintainer(),
        maintain_timer()
    {
        if (port) success = initialize(port);
        else
        {
            for (; lowport < 60000; ++lowport)
            {
                success = initialize(lowport);
                if (success) break;
            }
            if (!success) return;
        }

        maintain_timer.lock(); // Initialize timer.
        maintainer = std::thread(&Slave::maintain_forever, this);
            // Start maintainer thread.
    }

    ~Slave ()
    {
        maintain_timer.unlock();
        if (maintainer.joinable())
            maintainer.join();
    }

    /*
        Allow conversion to (void *) to test if opened successfully.
    */
    operator void * () { return (void *) success; }

    /*
        Get the port this slave is serving on.
    */
    unsigned short port () { return myport; }

    /*
        Add a master.
    */
    void add_master (const std::string & hostname, unsigned short port);

    bool serve (SlaveJob & job);

};



}

#endif // !defined __slave_hpp__

