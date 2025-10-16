#ifndef CLIENT_STUB_HPP
#define CLIENT_STUB_HPP

#include "httpctx.hpp"

using namespace boost;
namespace ssl = boost::asio::ssl;

namespace granada
{
    namespace http
    {

        template <class T>
        extern void onResolve(const boost::system::error_code &, tcp::resolver::results_type endpoints, const HttpContextPtr<T> &);
        extern template void onResolve<http::sSock>(const boost::system::error_code &, tcp::resolver::results_type endpoints, const HttpContextPtr<http::sSock> &);
        extern template void onResolve<http::tSock>(const boost::system::error_code &, tcp::resolver::results_type endpoints, const HttpContextPtr<http::tSock> &);

        template <class T>
        extern void onConnect(const boost::system::error_code &, const HttpContextPtr<T> &);

        extern void onHandshake(const boost::system::error_code &, const HttpContextPtr<sSock> &);

        template <class T>
        extern void onWrite(const boost::system::error_code &, std::size_t bytes_transferred, const HttpContextPtr<T> &);
        extern template void onWrite<http::sSock>(const boost::system::error_code &, std::size_t bytes_transferred, const HttpContextPtr<http::sSock> &);
        extern template void onWrite<http::tSock>(const boost::system::error_code &, std::size_t bytes_transferred, const HttpContextPtr<http::tSock> &);

        template <class T>
        extern void onReadStatusLine(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<T> &, ResponsePtr);
        extern template void onReadStatusLine<http::sSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::sSock> &, ResponsePtr);
        extern template void onReadStatusLine<http::tSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::tSock> &, ResponsePtr);

        template <class T>
        extern void onReadHeaders(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<T> &, ResponsePtr);
        extern template void onReadHeaders<http::sSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::sSock> &, ResponsePtr);
        extern template void onReadHeaders<http::tSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::tSock> &, ResponsePtr);

        template <class T>
        extern void onReadIdentityBody(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<T> &, ResponsePtr);
        extern template void onReadIdentityBody<http::sSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::sSock> &, ResponsePtr);
        extern template void onReadIdentityBody<http::tSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::tSock> &, ResponsePtr);

        template <class T>
        extern void readChunkSize(const HttpContextPtr<T> &, ResponsePtr);
        extern template void readChunkSize<http::sSock>(const HttpContextPtr<http::sSock> &, ResponsePtr);
        extern template void readChunkSize<http::tSock>(const HttpContextPtr<http::tSock> &, ResponsePtr);

        template <class T>
        extern void readChunkBody(const std::size_t chunk_size, const HttpContextPtr<T> &, ResponsePtr);
        extern template void readChunkBody<http::sSock>(const std::size_t chunk_size, const HttpContextPtr<http::sSock> &, ResponsePtr);
        extern template void readChunkBody<http::tSock>(const std::size_t chunk_size, const HttpContextPtr<http::tSock> &, ResponsePtr);

        extern void finish(HttpBasicContextPtr context, const boost::system::error_code &, ResponsePtr);
    }
}

#endif