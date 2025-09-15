#ifndef ENCODE_HPP
#define ENCODE_HPP

#include <string>

namespace granada
{
    extern const std::string base64Encode(const std::string &);
    extern const std::string urlEncode(const std::string &);
    extern const std::string hmacSha256(const std::string &key, const std::string &data);
}

#endif