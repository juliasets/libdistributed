
#include "Slave.hpp"

#include "utility.hpp"
#include "utility_macros.hpp"
#include "streamwrapper.hpp"

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
    for (unsigned pause = 100;
        // Workaround: try_lock_for() is broken in gcc 4.7, and won't be
        // fixed until gcc 4.9.
        // See gcc.gnu.org/bugzilla/show_bug.cgi?id=54562
        // Workaround: try_lock_until() is broken in gcc 4.8, so use
        // gcc-4.7 in the makefile.
        !maintain_timer.try_lock_until(
            std::chrono::steady_clock::now() +
                std::chrono::milliseconds(pause));
        pause = masters.size() > 0 ? 10000 : 100
    )
    {
        SYNCHRONIZED (master_lock)
        {
            for (const _master & master : masters)
            {
                try
                {
                    boost::asio::ip::tcp::iostream stream(master.hostname,
                        std::to_string(master.port),
                        boost::asio::ip::resolver_query_base::flags(0));
                    if (!stream) continue;
                    _utility::StreamWrapper wrapped(_key, stream);
                    _utility::log.o << "Adding self (" << myport << ") to (" <<
                        master.hostname << ", " << master.port
                        << ")" << std::endl;
                    _utility::log.flush();
                    wrapped.o << "slave" << ' ' << myport << ' ' << load() <<
                        std::endl;
                    wrapped.flush();
                }
                catch (std::exception & e)
                {
                    _utility::log.o << "Exception in Slave::maintain_forever(): " <<
                        e.what() << std::endl;
                    _utility::log.flush();
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


bool Slave::serve (SlaveJob & job)
{
    try
    {
        boost::system::error_code error;
        boost::asio::ip::tcp::iostream stream;
        boost::asio::ip::tcp::endpoint remote_ep;
        acceptor.accept(*stream.rdbuf(), remote_ep, error);
        _utility::log.o << "Slave accepted" << std::endl;
        _utility::log.flush();
        if (error) return false;
        _utility::StreamWrapper wrapped(_key, stream);
        wrapped.buffer();
        unsigned short port;
        wrapped.i >> port;
        wrapped.i.get(); // Eat one whitespace after port.
        std::stringstream data;
        data << wrapped.i.rdbuf();
        job = SlaveJob(_key,
            remote_ep.address().to_string(), port, data.str());
        return true;
    }
    catch (std::exception & e)
    {
        _utility::log.o << "Exception in Slave::serve_forever(): " <<
            e.what() << std::endl;
        _utility::log.flush();
        return false;
    }
}


