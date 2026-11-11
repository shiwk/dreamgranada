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
        void onReadStatusLine(const boost::system::error_code &error, const std::size_t readSize, const HttpContextPtr<T> &context, ResponsePtr response)
        {
            if (error)
            {
                LOG_ERROR_FMT("Read status line error: {}", error.message());
                return finish(context, error, nullptr);
            }

            LOG_DEBUG_FMT("Read status line done. {} bytes", readSize);

            // copy and remove characters from the resp buf
            auto &buffer = context->respBuff;
            LOG_DEBUG_FMT("buffer size: len({})", buffer.size());
            StatusLine statueLine(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + readSize);
            LOG_DEBUG_FMT("status line: {}", statueLine);
            buffer.consume(readSize);

            http::parseStatusLine(statueLine, response);

            // read the response headers
            asio::async_read_until(*context->sock, buffer, "\r\n\r\n",
                                   [context, response](const error_code &error, std::size_t size)
                                   {
                                       onReadHeaders(error, size, context, response);
                                   });
        }

        template <class T>
        void onReadHeaders(const boost::system::error_code &error, const std::size_t readSize, const HttpContextPtr<T> &context, ResponsePtr response)
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

            LOG_DEBUG_FMT("read header: len({})", readSize);
            LOG_DEBUG_FMT("buffer size: len({})", buffer.size());

            if (!http::shouldReadBody(response))
            {
                LOG_INFO("Read response done with empty body.");
                return finish(context, asio::error::eof, response);
            }

            if (response->chunked())
            {
                readChunkedBody(context, response);
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
        void onReadIdentityBody(const boost::system::error_code &error, const std::size_t readSize, const HttpContextPtr<T> &context, ResponsePtr response)
        {
            if (error && error != asio::error::eof)
            {
                LOG_ERROR_FMT("Read body error: {}", error.message());
                finish(context, error, nullptr);
                return;
            }

            LOG_DEBUG_FMT("read body length: {}, ec msg:{}", readSize, error.message());

            auto &buffer = context->respBuff;

            if (readSize > 0 || buffer.size() > 0)
            {
                auto bufs = buffer.data();
                auto it = buffer_sequence_begin(bufs);
                auto end = buffer_sequence_end(bufs);

                for (; it != end; ++it)
                {
                    const auto &buf = *it;
                    const char *p = static_cast<const char *>(buf.data());
                    std::size_t n = buf.size();
                    response->content->append(p, n);
                }
                buffer.consume(buffer.size());
            }

            if (error == asio::error::eof)
            {
                LOG_INFO("Read response done");
                return finish(context, asio::error::eof, response);
            }

            // continue read
            return asio::async_read(*context->sock, buffer, boost::asio::transfer_at_least(1),
                                    [context, response](const error_code &err, std::size_t size)
                                    {
                                        onReadIdentityBody(err, size, context, response);
                                    });
        }

        template <class T>
        void drainChunks(const HttpContextPtr<T> &context, ResponsePtr response, bool streamEnded);

        template <class T>
        void readMoreChunkData(const HttpContextPtr<T> &context, ResponsePtr response)
        {
            auto &buffer = context->respBuff;
            asio::async_read(*context->sock, buffer, asio::transfer_at_least(1),
                             [context, response](const error_code &error, std::size_t)
                             {
                                 if (error == asio::error::eof)
                                 {
                                     // The peer closed the stream. Decode whatever is
                                     // already buffered before finishing: the final
                                     // chunks may have arrived together with the FIN.
                                     return drainChunks(context, response, true);
                                 }
                                 if (error)
                                 {
                                     LOG_ERROR_FMT("Read body error: {}", error.message());
                                     return finish(context, error, nullptr);
                                 }
                                 drainChunks(context, response, false);
                             });
        }

        // Decodes every complete chunk currently buffered. Reads more data only
        // when a chunk is incomplete and the stream is still open; once the
        // stream has ended, buffered data is decoded before completing.
        template <class T>
        void drainChunks(const HttpContextPtr<T> &context, ResponsePtr response, bool streamEnded)
        {
            auto &buffer = context->respBuff;
            for (;;)
            {
                const std::string data(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_end(buffer.data()));

                const std::size_t lineEnd = data.find("\r\n");
                if (lineEnd == std::string::npos)
                {
                    if (streamEnded)
                    {
                        LOG_INFO("Read chunk to end.");
                        return finish(context, asio::error::eof, response);
                    }
                    return readMoreChunkData(context, response);
                }

                std::size_t chunkSize = 0;
                try
                {
                    // Trailing chunk extensions after ';' are ignored by stoul.
                    chunkSize = std::stoul(data.substr(0, lineEnd), nullptr, 16);
                }
                catch (const std::exception &e)
                {
                    // A malformed chunk header leaves no pending read, so
                    // without completing here the request would hang.
                    LOG_ERROR_FMT("Invalid chunk size: {}, body: {}", e.what(), data.substr(0, lineEnd));
                    return finish(context, asio::error::invalid_argument, nullptr);
                }

                if (chunkSize == 0)
                {
                    LOG_DEBUG("Read chunk to end.");
                    return finish(context, asio::error::eof, response);
                }

                const std::size_t chunkTotal = lineEnd + 2 + chunkSize + 2; // size line + content + CRLF
                if (data.size() < chunkTotal)
                {
                    if (streamEnded)
                    {
                        LOG_ERROR_FMT("Truncated chunk: expected {} bytes, have {}", chunkTotal, data.size());
                        return finish(context, asio::error::eof, response);
                    }
                    return readMoreChunkData(context, response);
                }

                response->content->append(data.data() + lineEnd + 2, chunkSize);
                buffer.consume(chunkTotal);
            }
        }

        template <class T>
        void readChunkedBody(const HttpContextPtr<T> &context, ResponsePtr response)
        {
            drainChunks(context, response, false);
        }

        void finish(HttpBasicContextPtr context, const error_code &error, ResponsePtr response)
        {
            context->complete(error, response);
        }

        template void onResolve<http::sSock>(const boost::system::error_code &, tcp::resolver::results_type endpoints, const HttpContextPtr<http::sSock> &);
        template void onResolve<http::tSock>(const boost::system::error_code &, tcp::resolver::results_type endpoints, const HttpContextPtr<http::tSock> &);
        template void onWrite<http::sSock>(const boost::system::error_code &, std::size_t bytes_transferred, const HttpContextPtr<http::sSock> &);
        template void onWrite<http::tSock>(const boost::system::error_code &, std::size_t bytes_transferred, const HttpContextPtr<http::tSock> &);
        template void onReadStatusLine<http::sSock>(const boost::system::error_code &, const std::size_t readSize, const HttpContextPtr<http::sSock> &, ResponsePtr);
        template void onReadStatusLine<http::tSock>(const boost::system::error_code &, const std::size_t readSize, const HttpContextPtr<http::tSock> &, ResponsePtr);
        template void onReadHeaders<http::sSock>(const boost::system::error_code &, const std::size_t readSize, const HttpContextPtr<http::sSock> &, ResponsePtr);
        template void onReadHeaders<http::tSock>(const boost::system::error_code &, const std::size_t readSize, const HttpContextPtr<http::tSock> &, ResponsePtr);
        template void onReadIdentityBody<http::sSock>(const boost::system::error_code &, const std::size_t readSize, const HttpContextPtr<http::sSock> &, ResponsePtr);
        template void onReadIdentityBody<http::tSock>(const boost::system::error_code &, const std::size_t readSize, const HttpContextPtr<http::tSock> &, ResponsePtr);
        template void readChunkedBody<http::sSock>(const HttpContextPtr<http::sSock> &, ResponsePtr);
        template void readChunkedBody<http::tSock>(const HttpContextPtr<http::tSock> &, ResponsePtr);
    };
}
