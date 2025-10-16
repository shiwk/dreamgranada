#ifndef ENCODE_HPP
#define ENCODE_HPP

#include <string>

namespace granada
{
    namespace utils
    {
        namespace encode
        {
            extern const std::string base64Encode(const std::string &);
            extern const std::string urlEncode(const std::string &);
            extern const std::string hmacSha256(const std::string &key, const std::string &data);
            extern const std::string sha256(const std::string &);
            extern const std::string toHex(const std::string &, bool lowerCase = false);
            extern const char *upperHexDigits;
            extern const char *lowerHexDigits;
        }
    }
}

#endif