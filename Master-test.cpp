
#include "Master.hpp"

#include "utility.hpp"

#include <iostream>

using namespace Distributed;


int main ()
{
    Master master;
    _utility::log.o << "Master: " << master.port() << std::endl;
    _utility::log.flush();
    std::cout << master.port() << std::endl;
    master.serve_forever();
}


