#ifndef CLIENTS3_HPP
#define CLIENTS3_HPP

#include "httpctx.hpp"
#include "logger.hpp"

using namespace boost;
namespace ssl = boost::asio::ssl;


namespace granada
{
    namespace http
    {
        class HttpClient3
        {
        private:
            static void onResolve(const error_code &error, tcp::resolver::results_type endpoints, const HttpContextPtr&);
            static void onConnect(const error_code &error, const HttpContextPtr&);
            static void onHandshake(const error_code &error, const HttpContextPtr&);
            static void onWrite(const error_code &error, std::size_t bytes_transferred, const HttpContextPtr&);
            static void onReadStatusLine(const error_code &error, const std::size_t size, const HttpContextPtr&);
            static void onReadHeaders(const error_code &error, const std::size_t size, const HttpContextPtr&);
            static void onReadBody(const error_code &error, const std::size_t size, const HttpContextPtr&);

        public:
            static void asyncRequest(io_contextPtr&, RequestPtr &, const ResponseHandler &, const ErrorHandler &);
        };

    }
}

#endif