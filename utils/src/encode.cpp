#include "encode.hpp"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

const std::string granada::utils::encode::base64Encode(const std::string &original)
{
    // LOG_INFO_FMT("base64Encode original: {}", original);
    BIO *bio = BIO_new(BIO_s_mem());
    BIO *base64 = BIO_new(BIO_f_base64());

    // link BIO
    bio = BIO_push(base64, bio);
    BIO_set_flags(base64, BIO_FLAGS_BASE64_NO_NL);
    
    // write data
    BIO_write(bio, original.data(), original.size());
    BIO_flush(bio);

    BUF_MEM *buffer_ptr;
    BIO_get_mem_ptr(bio, &buffer_ptr);

    std::string result(buffer_ptr->data, buffer_ptr->length);

    // free
    BIO_free_all(bio);

    return result;
}

const std::string granada::utils::encode::urlEncode(const std::string &src)
{
    static const char* hex = "0123456789ABCDEF";
    std::string output;
    output.reserve(src.size() * 3);

    for (unsigned char c : src)
    {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            output += c;
        }
        else
        {
            output += '%';
            output += hex[c >> 4];
            output += hex[c & 15];
        }
    }

    return output;
}

const std::string granada::utils::encode::hmacSha256(const std::string &key, const std::string &data)
{
    unsigned int len = SHA256_DIGEST_LENGTH;
    std::vector<unsigned char> result(len);
    HMAC(EVP_sha256(), key.c_str(), key.length(),
         reinterpret_cast<const unsigned char *>(data.c_str()), data.length(),
         result.data(), &len);

    return std::string(result.begin(), result.begin() + len);
}

const std::string granada::utils::encode::sha256(const std::string &data)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(data.c_str()), data.length(), hash);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

const std::string granada::utils::encode::toHex(const std::string &input, bool lowerCase)
{
    std::string output;
    output.reserve(input.size() * 2);
    for (unsigned char c : input)
    {
        if (lowerCase)
        {
            output.push_back(lowerHexDigits[c >> 4]);
            output.push_back(lowerHexDigits[c & 15]);
        }
        else
        {
            output.push_back(upperHexDigits[c >> 4]);
            output.push_back(upperHexDigits[c & 15]);
        }
    }
    return output;
}

const char* granada::utils::encode::upperHexDigits = "0123456789ABCDEF";
const char* granada::utils::encode::lowerHexDigits = "0123456789abcdef";
