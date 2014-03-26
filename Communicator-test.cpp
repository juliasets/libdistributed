
#include "Communicator.hpp"

#include <iostream>
#include <cstring>


using namespace Distributed;
using namespace Distributed::_utility;


int first (void) {
    Communicator comm;
    if (!comm)
    {
        std::cerr << "Failed to create communicator in first instance." <<
            std::endl;
        return 1;
    }
    std::cout << comm.port() << std::endl;
    bool success;
    std::string hostname;
    unsigned short other_port;
    std::string msg;
    success = comm.recv_from(hostname, other_port, msg);
    if (!success)
    {
        std::cerr << "Failed to receive message from second instance." <<
            std::endl;
        return 1;
    }
    if (msg != "hello\nworld")
    {
        std::cerr << "Message received incorrectly from second instance." <<
            std::endl;
        return 1;
    }
    std::cout << other_port << std::endl;
    success = comm.send_to(hostname, other_port, msg);
    if (!success)
    {
        std::cerr << "Failed to send message to second instance." <<
            std::endl;
        return 1;
    }
    return 0;
}


int second (void) {
    Communicator comm;
    if (!comm)
    {
        std::cerr << "Failed to create communicator in first instance." <<
            std::endl;
        return 1;
    }
    unsigned short other_port;
    std::cin >> other_port;
    bool success;
    success = comm.send_to("localhost", other_port, "hello\nworld");
    if (!success)
    {
        std::cerr << "Failed to send message to first instance." <<
            std::endl;
        return 1;
    }
    unsigned short my_port;
    std::cin >> my_port;
    if (my_port != comm.port()) {
        std::cerr << "Something evil happened with port numbers." << std::endl;
        return 1;
    }
    std::string hostname, msg;
    success = comm.recv_from(hostname, other_port, msg);
    if (!success)
    {
        std::cerr << "Failed to receive message from first instance." <<
            std::endl;
        return 1;
    }
    if (msg != "hello\nworld")
    {
        std::cerr << "Message received incorrectly from first instance." <<
            std::endl;
        return 1;
    }
    return 0;
}


int main (int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " first | second" << std::endl;
        return 1;
    }
    if (strcmp(argv[1], "second") == 0)
        return second();
    return first();
}


