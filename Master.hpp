
#if !defined __master_hpp__
#define __master_hpp__

#include "Communicator.hpp"

#include <vector>
#include <utility>
#include <sstream>
#include <mutex>

namespace Distributed {



class Master
{

    _utility::Communicator comm;

    struct _master
    {
        std::string hostname;
        unsigned short port;
        bool alive;
    };

    struct _slave
    {
        std::string hostname;
        unsigned short port;
        bool alive;
        double load;
    };

    struct _client
    {
        std::string hostname;
        unsigned short port;
    };

    struct _job
    {
        std::string msg;
        _slave assignee;
        _client assigner;
    };

    typedef host std::pair<std::string, unsigned short>;
    std::vector<_master> masters;
    std::vector<_slave> slaves;
    std::vector<_job> jobs;
    ssize_t my_place = 0;

    std::recursive_mutex master_lock, slave_lock, job_lock;

    _slave get_slave ();

    send_masters (const std::string & hostname, unsigned short port);

public:

    Master (unsigned short port = 0) :
        comm(port)
    {}

    operator void * () { return (void *) comm; }

    /*
        Get the port this master is serving on.
    */
    unsigned short port () { return comm.port(); }

    /*
        Inform an existing master of my existence. I don't become a secondary
        until I hear back from an existing master.
    */
    bool find_master (const std::string & hostname, unsigned short port)
    {
        return comm.send_to(hostname, port, "Master_join\n");
    }

    void serve_forever ();

};



}

#endif // !defined __master_hpp

