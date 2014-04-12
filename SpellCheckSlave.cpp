
#include "Slave.hpp"

#include "utility.hpp"

#include "../SpellCorrector/threadedSpellCorrector.h"

#include "ThreadPool.hpp"

#include <iostream>
#include <sstream>

using namespace Distributed;

using namespace SpellCorrector;


int main ()
{
    unsigned short masterport;
    std::cin >> masterport;
    std::cout << masterport << std::endl; // Pass master port to next slave.

    Slave slave;
    slave.add_master("localhost", masterport);

    _utility::log.o << "Slave: " << slave.port() << std::endl;
    _utility::log.flush();
    
    ThreadPool tpool (4);
    
    corrector * corr = new corrector();
    
    corr->loadDictionary("../Dictionary/unigrams.txt");
    corr->loadErrors("../Dictionary/trained21.txt");
    
    sqlite3 *db;
    int rc;
    rc = sqlite3_open("../Dictionary/BigramDatabase.db", &db);
    if (rc)
    {
        std::cout<<"Can't open database"<<std::endl;
        return 1;
    }
    
    std::string input;
    std::string output;
    std::string first;
    std::stringstream ss;
    
    std::string result;

    for (;;)
    {
        SlaveJob job;
        if (slave.serve(job))
        {
            _utility::log.o << "Received job." << std::endl;
            _utility::log.flush();
            
            ss.str(std::string());
            result = "";
            first = cmd_begin; //macro defined in threadedSpellCorrector.h
            
            ss << job.get_job();
            
            while (ss >> input)
            {
                output = correct(input, corr, first, db);
                
                std::stringstream ss2 (output);
                result = result + output + " ";
                while (ss2 >> first);
            }
            
            job.send_result(result);
        }
    }
    
    tpool.shutdown();
}


