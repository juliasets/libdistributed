
#if !defined __master_hpp__
#define __master_hpp__

#include <vector>
#include <utility>
#include <mutex>
#include <boost/asio.hpp>

namespace Distributed {



class Master
{

    struct _slave
    {
        std::string hostname;
        unsigned short port;
        double load;
        time_t last_seen;
    };

    std::vector<_slave> slaves;

    std::recursive_mutex slave_lock;

    std::string _key;

    static unsigned short lowport;
    unsigned short myport;
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    bool success = false;

    bool initialize (unsigned short port);

    bool get_slave (_slave & slave);

public:

    Master (const Master &) = delete;

    Master (const std::string & key, unsigned short port = 0) :
        _key(key),
        io_service(),
        acceptor(io_service)
    {
        if (port) success = initialize(port);
        else
        {
            for (; lowport < 60000; ++lowport)
            {
                success = initialize(lowport);
                if (success) break;
            }
        }
    }

    /*
        Allow conversion to (void *) to test if opened successfully.
    */
    operator void * () { return (void *) success; }

    /*
        Get the port this master is serving on.
    */
    unsigned short port () { return myport; }

    void serve_forever ();

};



}

#endif // !defined __master_hpp

