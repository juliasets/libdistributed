
#include "Slave.hpp"

#include "utility.hpp"

#include <iostream>
#include <sstream>

using namespace Distributed;


int main ()
{
    unsigned short masterport;
    std::cin >> masterport;
    std::cout << masterport << std::endl; // Pass master port to next slave.

    Slave slave;
    slave.add_master("localhost", masterport);

    _utility::log.o << "Slave: " << slave.port() << std::endl;
    _utility::log.flush();

    for (;;)
    {
        SlaveJob job;
        if (slave.serve(job))
        {
            _utility::log.o << "Received job: " << job.get_job() <<
                std::endl;
            _utility::log.flush();
            job.send_result("Thanks for the job!");
        }
    }
}


