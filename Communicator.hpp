
#if !defined __communicator_hpp__
#define __communicator_hpp__

#include <string>
#include <iostream>
#include <boost/asio.hpp>

namespace Distributed {
namespace _utility {



class Communicator
{

    static unsigned short lowport;

    unsigned short myport;
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    bool success = false;

    bool initialize (unsigned short port)
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

public:

    Communicator (const Communicator &) = delete;

    Communicator (unsigned short port = 0) :
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
    operator void * ()
    {
        return (void *) success;
    }

    unsigned short port () { return myport; }

    /*
        Reliably send a message to a recipient. Return true if the message
        was definitely received, otherwise return false.
    */
    bool send_to (const std::string & hostname, unsigned short port,
        const std::string & msg)
    {
        bool received = false;
        try
        {
            boost::asio::ip::tcp::iostream stream(hostname, std::to_string(port));
            if (!stream) return false;
            stream << myport << std::endl;
            stream << msg.size() << std::endl;
            stream << msg << std::endl;
            stream >> received;
            return received;
        }
        catch (std::exception & e)
        {
            std::cerr << "Exception in send_to(\"" << hostname << "\", " <<
                port << ", \"...\"): " << e.what() << std::endl;
            return false;
        }
    }

    /*
        Receive a message and its return-address. Return false if there was
        an error.
    */
    bool recv_from (std::string & hostname, unsigned short & port,
        std::string & msg)
    {
        try
        {
            boost::system::error_code error;
            boost::asio::ip::tcp::iostream stream;
            boost::asio::ip::tcp::endpoint remote_ep;
            acceptor.accept(*stream.rdbuf(), remote_ep, error);
            if (error) return false;
            hostname = remote_ep.address().to_string();
            stream >> port;
            size_t len;
            stream >> len;
            stream.get(); // Eat one whitespace after length.
            msg.resize(len);
            stream.read(&msg[0], len);
            stream << true << std::endl;
            return true;
        }
        catch (std::exception & e)
        {
            std::cerr << "Exception in recv_from(\"" << hostname << "\", " <<
                port << ", \"...\"): " << e.what() << std::endl;
            return false;
        }
    }

};

unsigned short Communicator::lowport = 30000;



}
}

#endif // !defined __communicator_hpp__

