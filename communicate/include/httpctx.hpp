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

        struct HttpBasicContext : public std::enable_shared_from_this<HttpBasicContext>
        {
            HttpBasicContext(io_contextPtr &io_context, const RequestPtr &, ResponseHandler &&respHandler, ErrorHandler &&errorHandler);
            HttpBasicContext(const HttpBasicContext &) = delete;
            HttpBasicContext &operator=(const HttpBasicContext &) = delete;
            virtual ~HttpBasicContext();
            void timeout(uint64_t);
            void prepare(const http::RequestPtr &);
            void complete(const error_code &, ResponsePtr response);
            asio::streambuf reqBuff;
            asio::streambuf respBuff;

        protected:
            const io_contextPtr io_context_;
            ResponseHandler respHandler;
            ErrorHandler errorHandler;
            void dumpRequest();
            virtual void cleanUp() = 0;

        private:
            std::shared_ptr<asio::steady_timer> timer_;
        };

        MAKE_SHARED_PTR(HttpBasicContext);

        template <class T> 
        struct HttpContext : public HttpBasicContext
        {
            HttpContext(io_contextPtr &io_context, const RequestPtr &, ResponseHandler &&respHandler, ErrorHandler &&errorHandler);
            HttpContext(const HttpContext &) = delete;
            ~HttpContext() = default;
            HttpContext &operator=(const HttpContext &) = delete;
            std::shared_ptr<T> sock;

        protected:
            void cleanUp() override;
        };

        MAKE_SHARED_PTR_ALIAS_1(HttpContext);

        template <class T> 
        inline HttpContextPtr<T> createContext(io_contextPtr &io_context, RequestPtr &request, ResponseHandler &&respHandler, ErrorHandler &&errorHandler)
        {
            return std::make_shared<HttpContext<T>>(io_context, request, std::move(respHandler), std::move(errorHandler));
        }
        using tSock = tcp::socket;
        using sSock = ssl::stream<tSock>;
        extern void safeCloseSsl(std::shared_ptr<sSock> &);
        extern ssl::context &getSSLCtx();
    } // namespace http
} // namespace granada

#endif