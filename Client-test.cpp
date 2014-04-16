
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

    Client client("p455w0rd");
    client.add_master("127.0.0.1", masterport);

    std::vector< std::unique_ptr<ClientJob> > jobs;

    /*
        Create a bunch of jobs and send them to slaves.
    */
    for (unsigned i = 0; i < 10; ++i)
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
    for (auto & job : jobs)
    {
        std::string result;
        if (job->get_result(10000, result))
        {
            _utility::log.o << "ClientJob (" << job->port() <<
                ") result: " << result << std::endl;
        }
        else
        {
            _utility::log.o << "ClientJob (" << job->port() <<
                ") failed." << std::endl;
        }
        _utility::log.flush();
    }

    _utility::log.o << "Success!" << std::endl;
    _utility::log.flush();
}


