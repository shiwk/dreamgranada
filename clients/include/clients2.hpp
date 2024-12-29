#ifndef CLIENTS2_HPP
#define CLIENTS2_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "httpctx.hpp"
#include "logger.hpp"

using namespace boost;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;


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
            ~HttpClient2(){
                LOG_INFO( "HttpClient2 destroyed");
            };
            void asyncRequest(RequestPtr &, const ResponseHandler &, const ErrorHandler &);
        };

    }
}

#endif