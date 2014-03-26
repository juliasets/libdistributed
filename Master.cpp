
#include "Master.hpp"
#include "utility.hpp"
#include "utility_macros.hpp"

#include <algorithm>
#include <random>
#include <ctime>

using namespace Distributed;
using namespace Distributed::_utility;


unsigned short Master::lowport = 30000;


bool Master::initialize (unsigned short port)
{
    myport = port;
    try
    {
        acceptor = std::move(boost::asio::ip::tcp::acceptor(
            io_service,
            boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                port)
        ));
        return true;
    }
    catch (std::exception &) {}
    return false;
}


/*
    Return a slave randomly, weighted inversely by their workload.
*/
Master::_slave Master::get_slave ()
{
    static std::random_device rd;
    static std::default_random_engine rng(rd());
    double total = 0.;
    SYNCHRONIZED (slave_lock)
    {
        for (const _slave & slave : slaves)
            total += (1. - slave.load);
        std::uniform_real_distribution<double> dist(0., total);
        total = dist(rng);
        for (const _slave & slave : slaves)
        {
            total -= (1. - slave.load);
            if (total < 0.) return slave;
        }
        return slaves[slaves.size() - 1]; // Default: return last slave.
    }
    return slaves[0]; // UNREACHABLE
}


void Master::serve_forever ()
{
    // TODO: Secondary masters.
    std::string command;
    for (;;)
    {
        try
        {
            boost::system::error_code error;
            boost::asio::ip::tcp::iostream stream;
            boost::asio::ip::tcp::endpoint remote_ep;
            acceptor.accept(*stream.rdbuf(), remote_ep, error);
            if (error) continue;
            stream >> command;
            if (command == "slave")
            {
                _slave slave;
                slave.hostname = remote_ep.address().to_string();
                stream >> slave.port;
                stream >> slave.load;
                slave.last_seen = time(NULL);
                std::cout << "Logging slave (" << slave.hostname << ", " <<
                    slave.port << ", " << slave.load << ") at time " <<
                    slave.last_seen << std::endl;
                SYNCHRONIZED (slave_lock)
                {
                    bool found = false;
                    for (size_t i = 0; i < slaves.size(); ++i)
                    {
                        // Update entry if it exists.
                        auto &s = slaves[i];
                        if (slave.hostname == s.hostname &&
                            slave.port == s.port)
                        {
                            s = slave;
                            found = true;
                        }
                        // Erase old entries.
                        if (time(NULL) - slave.last_seen > 60)
                        {
                            slaves.erase(slaves.begin() + i);
                            --i; // Cancel ++i.
                        }
                    }
                    // Add entry if it doesn't exist.
                    if (!found)
                        slaves.push_back(slave);
                }
            }
            else if (command == "client")
            {
                _slave slave = get_slave();
                stream << slave.hostname << ' ' << slave.port << std::endl;
            }
        }
        catch (std::exception & e)
        {
            std::cerr << "Exception in serve_forever: " << e.what() <<
                std::endl;
            continue;
        }
    }
}


