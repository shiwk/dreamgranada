#ifndef REQRESP_HPP
#define REQRESP_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include "common.hpp"
#include <boost/asio.hpp>


#define HTTP "http"
#define HTTPS "https"
#define CHUNKED "chunked"
#define TRANSFER_ENCODING "Transfer-Encoding"

namespace granada
{
    namespace http
    {
        enum class Method
        {
            GET = 0,
            POST,
            PUT,
            DELETE,
            HEAD
        };

        struct Request
        {
            bool https = true;
            Method method;
            std::string path;
            std::string host;
            std::string connection;
            std::string user_agent;
            std::string body;
            std::unordered_map<std::string, std::string> headers;
            std::unordered_map<std::string, std::string> queries;
            int8_t timeout = 10;

            Request(const Method &method, const std::string &host, const std::string &path, const std::string &user_agent = "granada_client", const std::string &connection = "close", const std::string &body = "", const bool https = true);
            void addHeader(const std::string &, const std::string &value);
            void addQuery(const std::string &, const std::string &value = "");
        };

        MAKE_SHARED_PTR(Request);

        using HttpStatus = unsigned short;
        using ErrorHandler = std::function<void(const boost::system::error_code &)>;
        using RespHeaders = std::unordered_map<std::string, std::string>;
        using RespBody = std::string;

        struct ResponseStatus
        {
            ResponseStatus(HttpStatus code) : statusCode(code) {}
            std::string version;
            HttpStatus statusCode;
            std::string statusMessage;
        };
        MAKE_SHARED_PTR(ResponseStatus);

        struct Response : ResponseStatus
        {
            Response() : ResponseStatus(0) {};
            RespHeaders headers;
            RespBody content;
            bool chunked()
            {
                return headers.find(TRANSFER_ENCODING) != headers.end() && headers[TRANSFER_ENCODING] == CHUNKED;
            }
        };

        MAKE_SHARED_PTR(Response);

        using ResponseHandler = std::function<void(const boost::system::error_code &, ResponsePtr &)>;
        using HeaderLine = std::string;
        using StatusLine = std::string;
    }
}

#endif
