
#include "Client.hpp"

#include "utility.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <fstream>

#include <time.h>

using namespace Distributed;

bool eoph (std::string const &fullString, std::string const &endings)
{
    if (fullString.length() < 1)
        return false;
    char end = fullString[fullString.length() - 1];
    for (int i = 0; i < endings.length(); i++)
    {
        if (end == endings[i])
            return true;
    }
    return false;
}

std::string format_word(std::string word, bool * end)
{
    std::list<std::string> words;
    for (int i = 0; i < word.length(); i++)
    {
        int begin = i;
        while (ispunct(word[begin]))
            ++begin;
        if (begin >= word.length())
            break;
        i = begin + 1;
        while ((!ispunct(word[i])) && (i < word.length()))
            ++i;
        std::string tmpword = word.substr(begin, i - begin);
        words.push_back(tmpword);
        if (i < word.length())
            (*end) = true;
    }
    std::stringstream result;
    result << words.front();
    words.pop_front();
    while (!words.empty())
    {
        result << " " << words.front();
        words.pop_front();
    }
    return result.str();
}

int main ()
{
    unsigned short masterport;
    std::cin >> masterport;
    std::cout << masterport << std::endl; // Pass master port to next client.

    sleep(1);

    Client client;
    client.add_master("localhost", masterport);

    std::vector< std::unique_ptr<ClientJob> > jobs;
    
    std::string filename = "test.txt";
    std::ifstream in( filename.c_str(), std::ios_base::in);
    
    std::string phrase;
    std::string word;
    bool end;
    while (in>>word)
    {
        end = false;
        phrase = format_word(word, &end);
        while (!end)
        {
            in>>word;
            phrase += format_word(word, &end);
        }
        
        jobs.emplace_back(new ClientJob(client));
        if (!*jobs[i])
        {
            _utility::log.o << "Couldn't get slave from master." << std::endl;
            _utility::log.flush();
            return 1;
        }
        _utility::log.o << "ClientJob: " << jobs[i]->port() << std::endl;
        _utility::log.flush();
        if (!jobs[i]->send_job(phrase))
        {
            _utility::log.o << "Couldn't send job to slave." << std::endl;
            _utility::log.flush();
            return 1;
        }
    }
    
    std::string outfilename = "output.txt";
    std::ofstream out( outfilename.c_str(), std::ios_base::out);
    
    for (auto & job : jobs)
    {
        std::string result;
        if (job->get_result(10000, result))
        {
            out<<result<<" ";
            _utility::log.o << "ClientJob (" << job->port() <<
                ")" << std::endl;
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
