#include "client_stub.hpp"
#include "logger.hpp"

using namespace boost;
using namespace granada::http;

using namespace boost;
namespace ssl = boost::asio::ssl;

namespace granada
{
    namespace http
    {
        template <class T>
        void onResolve(const boost::system::error_code &error, tcp::resolver::results_type endpoints, const HttpContextPtr<T> &context)
        {
            if (error)
            {
                LOG_ERROR_FMT("Resolve error: {}", error.message());
                return finish(context, error, nullptr);
            }

            for (auto &endpoint : endpoints)
            {
                LOG_DEBUG_FMT("Endpoint: {}", endpoint.endpoint().address().to_string());
            }

            asio::async_connect(context->sock->lowest_layer(), endpoints, [context](const error_code &error, const tcp::endpoint &)
                                { onConnect(error, context); });
        }

        template <>
        void onConnect(const error_code &error, const HttpContextPtr<sSock> &context)
        {
            if (error)
            {
                LOG_ERROR_FMT("Connect error: {}", error.message());
                return finish(context, error, nullptr);
            }

            LOG_DEBUG("Client connected and handshaking..");
            context->sock->async_handshake(ssl::stream_base::client, [context](const error_code &error)
                                           { onHandshake(error, context); });
        }

        template <>
        void onConnect(const error_code &error, const HttpContextPtr<tSock> &context)
        {
            if (error)
            {
                LOG_ERROR_FMT("Connect error: {}", error.message());
                return finish(context, error, nullptr);
            }

            LOG_DEBUG("Skip handshake for HTTP and writing..");
            auto &buffer = context->reqBuff;
            asio::async_write(*context->sock, buffer, [context](const error_code &error, std::size_t bytes_transferred)
                              { onWrite(error, bytes_transferred, context); });
        }

        void onHandshake(const error_code &error, const HttpContextPtr<sSock> &context)
        {
            if (error)
            {
                LOG_ERROR_FMT("Handshake error: {}", error.message());
                ERR_print_errors_fp(stderr);
                unsigned long e;
                while ((e = ERR_get_error()) != 0)
                {
                    char buf[256];
                    ERR_error_string_n(e, buf, sizeof(buf));
                    LOG_ERROR_FMT("OpenSSL error: {}\n", buf);
                }
                return finish(context, error, nullptr);
            }

            LOG_DEBUG("Client handshaked and writing..");
            auto &buffer = context->reqBuff;
            asio::async_write(*context->sock, buffer, [context](const error_code &error, std::size_t bytes_transferred)
                              { onWrite(error, bytes_transferred, context); });
        }

        template <class T>
        void onWrite(const boost::system::error_code &error, std::size_t bytes_transferred, const HttpContextPtr<T> &context)
        {
            if (error)
            {
                LOG_ERROR_FMT("Write error: {}", error.message());
                return finish(context, error, nullptr);
            }

            LOG_DEBUG_FMT("Client wrote {} bytes", bytes_transferred);

            auto &buffer = context->respBuff;
            auto response = std::make_shared<Response>();

            // read http status line
            asio::async_read_until(*context->sock, buffer, "\r\n",
                                   [context, response](const error_code &error, std::size_t size)
                                   {
                                       onReadStatusLine(error, size, context, response);
                                   });
        }

        template <class T>
        void onReadStatusLine(const boost::system::error_code &error, const std::size_t read_size, const HttpContextPtr<T> &context, ResponsePtr response)
        {
            if (error)
            {
                LOG_ERROR_FMT("Read status line error: {}", error.message());
                return finish(context, error, nullptr);
            }

            LOG_DEBUG_FMT("Read status line done. {} bytes", read_size);

            // copy and remove characters from the resp buf
            auto &buffer = context->respBuff;
            LOG_DEBUG_FMT("buffer size: len({})", buffer.size());
            StatusLine statueLine(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + read_size);
            LOG_DEBUG_FMT("status line: {}", statueLine);
            buffer.consume(read_size);

            http::parseStatusLine(statueLine, response);

            // read the response headers
            asio::async_read_until(*context->sock, buffer, "\r\n\r\n",
                                   [context, response](const error_code &error, std::size_t size)
                                   {
                                       onReadHeaders(error, size, context, response);
                                   });
        }

        template <class T>
        void onReadHeaders(const boost::system::error_code &error, const std::size_t read_size, const HttpContextPtr<T> &context, ResponsePtr response)
        {
            if (error)
            {
                LOG_ERROR_FMT("Read http lint error: {}", error.message());
                return finish(context, error, nullptr);
            }

            // read response headers.
            auto &buffer = context->respBuff;
            std::istream response_stream(&buffer);
            std::string headerLine;

            while (std::getline(response_stream, headerLine) && headerLine != "\r")
            {
                // LOG_DEBUG_FMT("Header line: {}", headerLine);
                http::parseHeaderLine(headerLine, response);
            }

            LOG_DEBUG_FMT("read header: len({})", read_size);
            LOG_DEBUG_FMT("buffer size: len({})", buffer.size());

            if (!http::shouldReadBody(response))
            {
                LOG_INFO("Read response done with empty body.");
                return finish(context, asio::error::eof, response);
            }

            if (response->chunked())
            {
                readChunkSize(context, response);
            }
            else
            {
                asio::async_read(*context->sock, buffer, boost::asio::transfer_at_least(1),
                                 [context, response](const error_code &err, std::size_t size)
                                 {
                                     onReadIdentityBody(err, size, context, response);
                                 });
            }
        }

        template <class T>
        void onReadIdentityBody(const boost::system::error_code &error, const std::size_t read_size, const HttpContextPtr<T> &context, ResponsePtr response)
        {
            if (error && error != asio::error::eof)
            {
                LOG_ERROR_FMT("Read body error: {}", error.message());
                finish(context, error, nullptr);
                return;
            }

            LOG_DEBUG_FMT("read body length: {}, ec msg:{}", read_size, error.message());
            if (error == asio::error::eof)
            {
                LOG_INFO("Read response done");
                return finish(context, asio::error::eof, response);
            }

            auto &buffer = context->respBuff;
            RespBody body(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + buffer.size());
            buffer.consume(buffer.size());
            LOG_DEBUG_FMT("RespBody: {}", body);
            response->content.append(body);

            // continue read
            return asio::async_read(*context->sock, buffer, boost::asio::transfer_at_least(1),
                                    [context, response](const error_code &err, std::size_t size)
                                    {
                                        onReadIdentityBody(err, size, context, response);
                                    });
        }

        template <class T>
        void readChunkSize(const HttpContextPtr<T> &context, ResponsePtr response)
        {
            auto &buffer = context->respBuff;
            asio::async_read_until(*context->sock, buffer, "\r\n",
                                   [context, response](const error_code &error, std::size_t read_size)
                                   {
                                       if (error && error != asio::error::eof)
                                       {
                                           LOG_ERROR_FMT("Read body error: {}", error.message());
                                           return finish(context, error, nullptr);
                                       }

                                       if (error == asio::error::eof)
                                       {
                                           LOG_INFO("Read chunk to end.");
                                           return finish(context, asio::error::eof, response);
                                       }

                                       auto &buffer = context->respBuff;
                                       RespBody body(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + read_size);
                                       buffer.consume(read_size);
                                       try
                                       {
                                           size_t chunk_size = std::stoul(body, nullptr, 16);
                                           if (chunk_size == 0)
                                           {
                                               LOG_DEBUG_FMT("To read chunk size: {}", chunk_size);
                                               //    LOG_INFO("Read chunk to end.");
                                               return finish(context, asio::error::eof, response);
                                           }
                                           readChunkBody(chunk_size, context, response);
                                       }
                                       catch (const std::exception &e)
                                       {
                                           LOG_ERROR_FMT("Caught exception: {}, {}", e.what(), body);
                                       }
                                   });
        }

        template <class T>
        void readChunkBody(const std::size_t chunk_size, const HttpContextPtr<T> &context, ResponsePtr response)
        {
            auto &buffer = context->respBuff;
            size_t to_read_size = chunk_size + 2; // content + "\r\n"
            asio::async_read(*context->sock, buffer, asio::transfer_at_least(to_read_size),
                             [context, chunk_size, response](const error_code &error, std::size_t read_size)
                             {
                                 try
                                 {
                                     if (error && error != asio::error::eof)
                                     {
                                         LOG_ERROR_FMT("Read body error: {}", error.message());
                                         return finish(context, error, nullptr);
                                     }
                                     auto &buffer = context->respBuff;
                                     RespBody body(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + chunk_size);
                                     LOG_DEBUG_FMT("read chunk size: {}", body.size());

                                     buffer.consume(chunk_size + 2);

                                     response->content.append(body);
                                     readChunkSize(context, response);
                                 }
                                 catch (const std::exception &e)
                                 {
                                     LOG_ERROR_FMT("Caught exception: {}", e.what());
                                 }
                             });
        }

        void finish(HttpBasicContextPtr context, const error_code &error, ResponsePtr response)
        {
            context->complete(error, response);
        }

        template void onResolve<http::sSock>(const boost::system::error_code &, tcp::resolver::results_type endpoints, const HttpContextPtr<http::sSock> &);
        template void onResolve<http::tSock>(const boost::system::error_code &, tcp::resolver::results_type endpoints, const HttpContextPtr<http::tSock> &);
        template void onWrite<http::sSock>(const boost::system::error_code &, std::size_t bytes_transferred, const HttpContextPtr<http::sSock> &);
        template void onWrite<http::tSock>(const boost::system::error_code &, std::size_t bytes_transferred, const HttpContextPtr<http::tSock> &);
        template void onReadStatusLine<http::sSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::sSock> &, ResponsePtr);
        template void onReadStatusLine<http::tSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::tSock> &, ResponsePtr);
        template void onReadHeaders<http::sSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::sSock> &, ResponsePtr);
        template void onReadHeaders<http::tSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::tSock> &, ResponsePtr);
        template void onReadIdentityBody<http::sSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::sSock> &, ResponsePtr);
        template void onReadIdentityBody<http::tSock>(const boost::system::error_code &, const std::size_t read_size, const HttpContextPtr<http::tSock> &, ResponsePtr);
        template void readChunkSize<http::sSock>(const HttpContextPtr<http::sSock> &, ResponsePtr);
        template void readChunkSize<http::tSock>(const HttpContextPtr<http::tSock> &, ResponsePtr);
        template void readChunkBody<http::sSock>(const std::size_t chunk_size, const HttpContextPtr<http::sSock> &, ResponsePtr);
        template void readChunkBody<http::tSock>(const std::size_t chunk_size, const HttpContextPtr<http::tSock> &, ResponsePtr);
    };
}
