
#include "Client.hpp"

#include "utility.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <memory>

#include <time.h>

using namespace Distributed;


int main ()
{
    unsigned short masterport;
    std::cin >> masterport;
    std::cout << masterport << std::endl; // Pass master port to next client.

    sleep(1);

    Client client;
    client.add_master("localhost", masterport);

    std::vector< std::unique_ptr<ClientJob> > jobs;

    /*
        Create a bunch of jobs and send them to slaves.
    */
    for (unsigned i = 0; i < 2; ++i)
    {
        jobs.emplace_back(new ClientJob(client));
        if (!*jobs[i])
        {
            _utility::log.o << "Couldn't get slave from master." << std::endl;
            _utility::log.flush();
            return 1;
        }
        _utility::log.o << "ClientJob: " << jobs[i]->port() << std::endl;
        _utility::log.flush();
        if (!jobs[i]->send_job(
            "hello world from " + std::to_string(jobs[i]->port()) + "!"))
        {
            _utility::log.o << "Couldn't send job to slave." << std::endl;
            _utility::log.flush();
            return 1;
        }
    }

    /*
        Get the result from each job.
    */
    for (unsigned i = 0; i < 1; ++i)
    {
        std::string result;
        if (jobs[i]->get_result(4000, result))
        {
            _utility::log.o << "ClientJob (" << jobs[i]->port() <<
                ") result: " << result << std::endl;
        }
        else
        {
            _utility::log.o << "ClientJob (" << jobs[i]->port() <<
                ") failed." << std::endl;
        }
        _utility::log.flush();
    }

}


