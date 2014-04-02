
#include "Slave.hpp"

#include "utility.hpp"

#include <iostream>
#include <sstream>

using namespace Distributed;


int main ()
{
    Slave slave;
    unsigned short masterport;
    std::cin >> masterport;
    std::cout << masterport << std::endl; // Pass master port to next slave.
    _utility::log.o << "Slave: " << slave.port() << std::endl;
    _utility::log.flush();
    slave.add_master("localhost", masterport);
    slave.serve();
}


