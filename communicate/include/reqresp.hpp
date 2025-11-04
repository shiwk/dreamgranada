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
#define HTTP_1_0 "HTTP/1.0"
#define CHUNKED "chunked"
#define TRANSFER_ENCODING "Transfer-Encoding"
#define Content_Length "Content-Length"
#define User_Agent "User-Agent"
#define Connection "Connection"

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

        using HeaderLine = std::string;
        using StatusLine = std::string;

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
            uint8_t timeout = 10;

            Request(const Method &method, const std::string &host, const std::string &path, const std::string &user_agent = "granada_client", const std::string &connection = "close", const std::string &body = "", const bool https = true);
            void addHeader(const std::string &, const std::string &value);
            void addQuery(const std::string &, const std::string &value = "");
        };

        MAKE_SHARED_PTR(Request);

        using HttpStatus = std::string;
        using HttpMessage = std::string;
        namespace status
        {
            extern HttpStatus OK;
        }
        using ErrorHandler = std::function<void(const boost::system::error_code &)>;
        using RespHeaders = std::unordered_map<std::string, std::string>;
        using RespBody = std::string;

        struct ResponseStatus
        {
            ResponseStatus() = default;
            std::string version;
            HttpStatus statusCode;
            HttpMessage statusMessage;
        };
        MAKE_SHARED_PTR(ResponseStatus);

        struct Response : ResponseStatus
        {
            Response() : ResponseStatus() {}
            RespHeaders headers;
            RespBody content;
            bool chunked()
            {
                return headers.find(TRANSFER_ENCODING) != headers.end() && headers[TRANSFER_ENCODING] == CHUNKED;
            }
        };

        MAKE_SHARED_PTR(Response);
        extern const std::string methodToString(const Method &);
        extern bool parseHeaderLine(const HeaderLine &, ResponsePtr &);
        extern bool parseStatusLine(const StatusLine &, ResponsePtr &);
        extern void writeQueryStream(const http::RequestPtr &request, std::ostream &stream, bool sort = false);
        using ResponseHandler = std::function<void(const boost::system::error_code &, ResponsePtr &)>;
        extern bool shouldReadBody(const http::ResponsePtr &);
    }
}

#endif
