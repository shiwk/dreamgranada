#ifndef HTTPCTX_HPP
#define HTTPCTX_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "common.hpp"
#include "reqresp.hpp"


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
        struct HttpContextUtil
        {
            static void split(const std::string &, const char, std::vector<std::string> &);
            static std::string strip(const std::string &str);
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
            void safeClose();
        };

        MAKE_SHARED_PTR(HttpContext);
    } // namespace http
} // namespace granada
#endif