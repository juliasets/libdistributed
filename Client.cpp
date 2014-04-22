
#include "Client.hpp"

#include "utility.hpp"
#include "streamwrapper.hpp"

#include <chrono>


using namespace Distributed;



/*
    Client:
*/


void Client::add_master (const std::string & hostname, unsigned short port)
{
    _master master;
    master.hostname = hostname;
    master.port = port;
    masters.push_back(master);
}


bool Client::get_slave (std::string & hostname, unsigned short & port) const
{
    for (_master master : masters)
    {
        try
        {
            boost::asio::ip::tcp::iostream
                stream(master.hostname, std::to_string(master.port),
                    boost::asio::ip::resolver_query_base::flags(0));
            if (!stream) continue;
            _utility::StreamWrapper wrapped(_key, stream);
            wrapped.o << "client" << std::endl;
            wrapped.flush();
            wrapped.buffer();
            wrapped.i >> hostname >> port;
            return true;
        }
        catch (std::exception & e)
        {
            _utility::log.o << "Exception in Client::get_slave(): " <<
                e.what() << std::endl;
            _utility::log.flush();
        }
    }
    return false;
}



/*
    ClientJob:
*/


bool ClientJob::initialize (unsigned short port)
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


void ClientJob::serve ()
{
    try
    {
        boost::system::error_code error;
        boost::asio::ip::tcp::iostream stream;
        boost::asio::ip::tcp::endpoint remote_ep;
        acceptor.accept(*stream.rdbuf(), remote_ep, error);
        if (error) throw std::exception();
        _utility::StreamWrapper wrapped(_key, stream);
        wrapped.buffer();
        std::stringstream data;
        data << wrapped.i.rdbuf();
        result = data.str();
    }
    catch (std::exception & e)
    {
        _utility::log.o << "Exception in ClientJob::serve(): " <<
            e.what() << std::endl;
        _utility::log.flush();
    }
    // Ensure lock is unlocked.
    _wait.unlock();
}


/*
    Open up a connection to myself in order to cancel wait.
    When this function returns, the lock _wait is unlocked.
*/
void ClientJob::cancel ()
{
    try
    {
        boost::asio::ip::tcp::iostream
            stream("127.0.0.1", std::to_string(myport),
                boost::asio::ip::resolver_query_base::flags(0));
        _utility::StreamWrapper wrapped(_key, stream);
        wrapped.flush();
    }
    catch (std::exception &e)
    {}
    _waiter.join();
}


/*
    Send port then message.
*/
bool ClientJob::send_job (const std::string & msg)
{
    try
    {
        boost::asio::ip::tcp::iostream
            stream(_hostname, std::to_string(_port),
                boost::asio::ip::resolver_query_base::flags(0));
        if (!stream) return false;
        _utility::StreamWrapper wrapped(_key, stream);
        wrapped.o << myport << std::endl;
        wrapped.o << msg;
        wrapped.flush();
        return true;
    }
    catch (std::exception & e)
    {
        return false;
    }
}


bool ClientJob::get_result (size_t ms, std::string & message)
{
    // Workaround: try_lock_for() is broken in gcc 4.7, and won't be
    // fixed until gcc 4.9.
    // See gcc.gnu.org/bugzilla/show_bug.cgi?id=54562
    // Workaround: try_lock_until() is broken in gcc 4.8, so use
    // gcc-4.7 in the makefile.
    if (
        !_wait.try_lock_until(
            std::chrono::steady_clock::now() +
                std::chrono::milliseconds(ms))
    ) // If we can't acquire the lock in time:
    {
        cancel(); // Cancel waiting for job. Handles releasing lock _wait.
        return false;
    }
    _wait.unlock(); // Release lock _wait.
    _waiter.join();
    message = result;
    return true;
}


