#include "encode.hpp"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <vector>

const std::string granada::base64Encode(const std::string &original)
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

const std::string granada::urlEncode(const std::string &src)
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

const std::string granada::hmacSha256(const std::string &key, const std::string &data)
{
    unsigned int len = SHA256_DIGEST_LENGTH;
    std::vector<unsigned char> result(len);
    HMAC(EVP_sha256(), key.c_str(), key.length(),
         reinterpret_cast<const unsigned char *>(data.c_str()), data.length(),
         result.data(), &len);

    return std::string(result.begin(), result.begin() + len);
}