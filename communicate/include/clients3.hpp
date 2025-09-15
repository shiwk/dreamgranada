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
            static void onResolve(const boost::system::error_code &error, tcp::resolver::results_type endpoints, const HttpContextPtr&, std::shared_ptr<asio::steady_timer>);
            static void onConnect(const boost::system::error_code &error, const HttpContextPtr&, std::shared_ptr<asio::steady_timer>);
            static void onHandshake(const boost::system::error_code &error, const HttpContextPtr&, std::shared_ptr<asio::steady_timer>);
            static void onWrite(const boost::system::error_code &error, std::size_t bytes_transferred, const HttpContextPtr&, std::shared_ptr<asio::steady_timer>);
            static void onReadStatusLine(const boost::system::error_code &error, const std::size_t size, const HttpContextPtr&, std::shared_ptr<asio::steady_timer>);
            static void onReadHeaders(const boost::system::error_code &error, const std::size_t size, const HttpContextPtr&, std::shared_ptr<asio::steady_timer>);
            static void onReadIdentityBody(const boost::system::error_code &error, const std::size_t size, const HttpContextPtr&, std::shared_ptr<asio::steady_timer>);
            static void readChunkSize(HttpContextPtr context, std::shared_ptr<asio::steady_timer>);
            static void readChunkBody(const std::size_t chunk_size, HttpContextPtr, std::shared_ptr<asio::steady_timer>);
            static void finish(HttpContextPtr context, const boost::system::error_code &error, std::shared_ptr<asio::steady_timer>);

        public:
            static void asyncRequest(io_contextPtr&, RequestPtr &, const ResponseHandler &, const ErrorHandler &);
        };

    }
}

#endif