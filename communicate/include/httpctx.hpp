#ifndef REQEUST_HPP
#define REQEUST_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "common.hpp"

#define HTTP "http"
#define HTTPS "https"
#define CHUNKED "chunked"
#define TRANSFER_ENCODING "Transfer-Encoding"

using namespace boost::asio;
namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
using namespace boost::system;

using asio::ip::tcp;

namespace granada
{
    namespace http
    {
        MAKE_SHARED_PTR(io_context);
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
        using ErrorHandler = std::function<void(const error_code &)>;
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

        using ResponseHandler = std::function<void(const error_code &, ResponsePtr &)>;
        using HeaderLine = std::string;
        using StatusLine = std::string;

        struct HttpContextUtil
        {
            static void split(const std::string &, const char, std::vector<std::string> &);
            static std::string strip(const std::string& str);
        };

        struct HttpContext : std::enable_shared_from_this<HttpContext>
        {
        private:
            io_contextPtr io_context_;
            int8_t timeout_;
            void prepareRequest(const RequestPtr &);
            void writeQueryStream(std::unordered_map<std::string, std::string> &, std::ostream &);
            void writeResqHeaders(std::unordered_map<std::string, std::string> &, std::ostream &);
            void cleanUp();
            // asio::steady_timer contextTimer_;

        public:
            HttpContext(io_contextPtr &io_context, RequestPtr &request, const ResponseHandler &respHandler, const ErrorHandler &errorHandler)
                : io_context_(io_context), timeout_(request->timeout), sock(*io_context, getSSLCtx()), reqBuff(), respBuff(), respHandler(respHandler), errorHandler(errorHandler), response(std::make_shared<Response>())
                // , contextTimer_(*io_context, std::chrono::seconds(10))
                {
                    prepareRequest(request);
                }

            HttpContext(const HttpContext &) = delete;
            ~HttpContext();
            HttpContext &operator=(const HttpContext &) = delete;
            void complete(const error_code &);
            void handleError(const error_code &);
            static ssl::context &getSSLCtx();

            asio::streambuf reqBuff;
            asio::streambuf respBuff;
            ResponsePtr response;
            ssl::stream<tcp::socket> sock;
            ResponseHandler respHandler;
            ErrorHandler errorHandler;
            StatusLine respStatusLine;

            static bool parseHeaderLine(const HeaderLine &, RespHeaders &);
            static bool parseStatusLine(const StatusLine &, ResponseStatusPtr);
            // static void startTimer(std::shared_ptr<HttpContext>);
            std::shared_ptr<asio::steady_timer> getTimer();
        private:
            // bool getResponse(ResponsePtr &);
            static bool parseHeaders(const std::vector<HeaderLine> &, RespHeaders &);
            void dumpRequest(RequestPtr request);
        };

        MAKE_SHARED_PTR(HttpContext);
    } // namespace http
} // namespace granada
#endif