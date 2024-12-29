#ifndef CLIENTS_HPP
#define CLIENTS_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "httpctx.hpp"

using namespace boost;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

#define HTTP "http"
#define HTTPS "https"
#define GET "GET"


namespace granada
{
    namespace http
    {
        class HttpClient2 : public boost::enable_shared_from_this<HttpClient2>
        {
        private:
            io_contextPtr io_context_;

        private:
            void onResolve(const error_code &error, tcp::resolver::results_type endpoints, const HttpContextPtr&);
            void onConnect(const error_code &error, const HttpContextPtr&);
            void onHandshake(const error_code &error, const HttpContextPtr&);
            void onWrite(const error_code &error, std::size_t bytes_transferred, const HttpContextPtr&);
            void onReadStatusLine(const error_code &error, const HttpContextPtr&);
            void onReadHeaders(const error_code &error, const HttpContextPtr&);
            void onReadBody(const error_code &error, const HttpContextPtr&);

        public:
            HttpClient2(io_contextPtr &io_context): io_context_(io_context) {};
            ~HttpClient2();
            void asyncRequest(RequestPtr &, const ResponseHandler &, const ErrorHandler &);
        };

    }
}

#endif