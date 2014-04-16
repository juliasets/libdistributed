
#include "Master.hpp"

#include "utility.hpp"

#include <iostream>

using namespace Distributed;


int main ()
{
    Master master("p455w0rd");

    _utility::log.o << "Master: " << master.port() << std::endl;
    _utility::log.flush();
    std::cout << master.port() << std::endl;

    master.serve_forever();
}


