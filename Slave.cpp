
#include "Slave.hpp"

#include "utility.hpp"
#include "utility_macros.hpp"

#include <chrono>
#include <sstream>

#include <cstdlib> // getloadavg


using namespace Distributed;


unsigned short Slave::lowport = 30000;


double Slave::load ()
{
    double ret;
    if (getloadavg(&ret, 1) == -1) return 0.5;
    return ret;
}


void Slave::maintain_forever ()
{
    for (;
        // Workaround: try_lock_for() is broken in gcc 4.7, and won't be
        // fixed until gcc 4.9
        // See gcc.gnu.org/bugzilla/show_bug.cgi?id=54562
        // Workaround: try_lock_until() is broken in gcc 4.8, so use
        // gcc-4.7 in the makefile.
        !maintain_timer.try_lock_until(
            std::chrono::steady_clock::now() +
                std::chrono::milliseconds(1000));
        )
    {
        SYNCHRONIZED (master_lock)
        {
            for (const _master & master : masters)
            {
                try
                {
                    boost::asio::ip::tcp::iostream stream(master.hostname,
                        std::to_string(master.port));
                    if (!stream) continue;
                    std::cout << "Adding self to (" << master.hostname <<
                        ", " << master.port << ")" << std::endl;
                    stream << "slave" << ' ' << myport << ' ' << load() <<
                        std::endl;
                }
                catch (std::exception &e)
                {
                    std::cerr << "Exception: " << e.what() << std::endl;
                }
            }
        }
    }
    maintain_timer.unlock(); // Avoid undefined behavior.
}


bool Slave::initialize (unsigned short port)
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


void Slave::add_master (const std::string & hostname, unsigned short port)
{
    _master master;
    master.hostname = hostname;
    master.port = port;
    SYNCHRONIZED (master_lock) masters.push_back(master);
}


std::string Slave::serve ()
{
    try
    {
        boost::system::error_code error;
        boost::asio::ip::tcp::iostream stream;
        boost::asio::ip::tcp::endpoint remote_ep;
        acceptor.accept(*stream.rdbuf(), remote_ep, error);
        if (error) return "";
        std::stringstream data;
        data << stream.rdbuf();
        return data.str();
    }
    catch (std::exception & e)
    {
        std::cerr << "Exception in serve_forever: " << e.what() <<
            std::endl;
        return "";
    }
}


