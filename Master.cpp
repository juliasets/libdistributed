
#include "Master.hpp"
#include "utility.hpp"

#include <algorithm>
#include <random>

using namespace Distributed;
using namespace Distributed::_utility;


/*
    Return a slave randomly, weighted inversely by their workload.
*/
Master::_slave Master::get_slave ()
{
    static std::random_device rd;
    static std::default_random_engine rng(rd());
    double total = 0.;
    SYNCHRONIZED (slave_lock)
    {
        for (const _slave & slave : slaves)
            total += (1. - slave.load);
        std::uniform_real_distribution<double> dist(0., total);
        total = dist(rng);
        for (const _slave & slave : slaves)
        {
            total -= (1. - slave.load);
            if (total < 0.) return slave;
        }
        return slaves[slaves.size() - 1]; // Default: return last slave.
    }
}


void Master::send_masters (const std::string & hostname, unsigned short port)
{
    std::stringstream ostream;
    SYNCHRONIZED (master_lock)
    {
        // Send master list, and where/if you are on it.
        ostream << "Master_list" << std::endl;
        ostream << masters.size() << std::endl;
        for (const _master & master : masters)
            ostream << master.hostname << ' ' << master.port << ' ' <<
                master.alive << ' ' <<
                (master.hostname == hostname && master.port == port) <<
                std::endl;
    }
    comm.send_to(hostname, port, ostream.str()); // Ignore errors here.
}


void Master::server_forever ()
{
    // TODO: spawn thread to poke the master above me, if one exists,
    // or poke clients if I am the master. Poke master by asking to join
    // it repeatedly.
    std::string hostname, msg, line;
    std::stringstream istream;
    unsigned short port;
    bool success;
    for (;;)
    {
        success = comm.recv_from(hostname, port, msg);
        if (!success) continue; // Failed to receive.
        istream.str(msg);
        istream >> line;
        if (line == "Master_join")
        {
            SYNCHRONIZED (master_lock)
            {
                // A master is trying to join. Add it to the list of
                // masters (if not already there).
                _master remote;
                remote.hostname = hostname;
                remote.port = port;
                remote.alive = true;
                if (std::find(masters.begin(), masters.end()) == masters.end())
                    masters.push_back(remote);
            }
            // Send master list.
            send_masters(hostname, port);
        }
        else if (line == "Master_list")
        {
            SYNCHRONIZED (master_lock)
            {
                // Receive master list, and where I am on it (if applicable).
                my_place = -1;
                size_t listsize;
                istream >> listsize;
                masters.resize(listsize);
                for (size_t i = 0; i < listsize; ++i)
                {
                    bool me;
                    _master remote;
                    istream >> remote.hostname >> remote.port >>
                        remote.alive >> me;
                    masters[i] = remote;
                    if (me) my_place = i;
                }
            }
            SYNCHRONIZED (slave_lock)
            {
                // Receive slave list.
                size_t listsize;
                istream >> listsize;
                slaves.resize(listsize);
                for (size_t i = 0; i < listsize; ++i)
                {
                    _slave remote;
                    istream >> remote.hostname >> remote.port >>
                        remote.alive >> remote.load;
                    slaves[i] = remote;
                }
            }
            SYNCHRONIZED (job_lock)
            {
                // Receive job list.
                size_t listsize;
                istream >> listsize;
                jobs.resize(listsize);
                for (size_t i = 0; i < listsize; ++i)
                {
                    _job remote;
                    istream >> 
                }
            }
        }
        else if (line == "Client_job")
        {
            std::stringstream ostream;
            ostream << "Master_job" << std::endl;
            istream >> ostream.rdbuf();
            _slave slave = get_slave();
            // If I can't send the job to a slave immediately, try to send
            // it back to the client. If that fails, give up hope.
            if (!comm.send_to(slave.hostname, slave.port, ostream.str()))
            {
                comm.send_to(hostname, port, ostream.str()); // Ignore errors.
                continue; // Don't add to job list.
            }
            _client client;
            client.hostname = hostname;
            client.port = port;
            _job job;
            istream >> job.msg;
            job.assignee = slave;
            job.assigner = client;
            SYNCHRONIZED (job_lock)
            {
                jobs.push_back(job);
            }
        }
        else if (line == "")
    }
}


