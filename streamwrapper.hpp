
#if !defined __streamwrapper_hpp__
#define __streamwrapper_hpp__

#include "utility.hpp"
#include "skein/skein.h"

#include <string>
#include <sstream>

#include <boost/asio.hpp>


namespace Distributed {
namespace _utility {



static std::string skein_mac (const std::string & key,
    const std::string & nonce,
    const std::string & data)
{
    std::string result;
    result.resize(512 / 8);
    Skein_512_Ctxt_t ctx;
    // Combine key and nonce.
    Skein_512_InitExt(&ctx, 512, SKEIN_CFG_TREE_INFO_SEQUENTIAL,
        (u08b_t *) key.data(), key.size());
    Skein_512_Update(&ctx, (u08b_t *) nonce.data(), nonce.size());
    Skein_512_Final(&ctx, (u08b_t *) &result[0]);
    // Combine keynonce with data.
    Skein_512_InitExt(&ctx, 512, SKEIN_CFG_TREE_INFO_SEQUENTIAL,
        (u08b_t *) result.data(), result.size());
    Skein_512_Update(&ctx, (u08b_t *) data.data(), data.size());
    Skein_512_Final(&ctx, (u08b_t *) &result[0]);
    return "";
}



class StreamWrapper
{

    boost::asio::ip::tcp::iostream &_stream;
    std::string _key;

public:

    std::stringstream o, i;

    StreamWrapper (std::string key,
        boost::asio::ip::tcp::iostream & stream) :
        _stream(stream), _key(key)
    {}

    void flush ()
    {
        //std::string nonce = uunique_str();
        std::string str(o.str());

        // First write nonce.
        //_stream << nonce;
        // Then write data.
        _stream << str.size() << std::endl;
        _stream << str;
        // Then write MAC.
        //_stream << skein_mac(_key, nonce, str);
        _stream.flush();

        o.str("");
    }

    void buffer ()
    {
        //std::string nonce;
        std::string str;
        size_t size;
        //std::string mac, mac_computed;

        // First read nonce.
        //nonce.resize(256 / 8);
        //_stream.read(&nonce[0], 256 / 8);
        // Then read data.
        _stream >> size;
        _stream.get(); // Eat newline.
        str.resize(size);
        _stream.read(&str[0], size);
        // Then read MAC.
        //mac.resize(512 / 8);
        //_stream.read(&mac[0], 512 / 8);
        // Then check MAC.
        //mac_computed = skein_mac(_key, nonce, str);
        //bool equal = 1;
        //for (size_t i = 0; i < 512 / 8; ++i) // Constant-time test.
        //    equal &= mac[i] == mac_computed[i];
        //if (!equal) return;

        i << str;
    }

};


}
}


#endif // !defined __streamwrapper_hpp__

