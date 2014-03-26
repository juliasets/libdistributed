
#include "Master.hpp"

#include <iostream>

using namespace Distributed;


int main ()
{
    Master master;
    std::cout << master.port() << std::endl;
    master.serve_forever();
}


