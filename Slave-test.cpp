
#include "Slave.hpp"

#include <iostream>

using namespace Distributed;


int main ()
{
    Slave slave;
    std::cout << slave.port() << std::endl;
    slave.add_master("localhost", 8989);
    slave.serve();
}


