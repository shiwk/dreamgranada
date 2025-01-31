#include "clients3.hpp"
#include "logger.hpp"

using namespace boost;
using namespace granada::http;

void HttpClient3::asyncRequest(io_contextPtr &io_context, RequestPtr &request, const ResponseHandler &requestCallback, const ErrorHandler &errorHandler)
{
    auto resolver = std::make_shared<tcp::resolver>(*io_context);
    auto context = std::make_shared<HttpContext>(io_context, request, requestCallback, errorHandler);
    LOG_INFO_FMT("Requesting {}://{}", request->https ? HTTPS : HTTP, request->host + request->path);

    resolver->async_resolve(request->host, request->https ? HTTPS : HTTP, [context, resolver](const error_code &err, tcp::resolver::results_type endpoints)
                            { onResolve(err, endpoints, context); });
}

void HttpClient3::onResolve(const error_code &error, tcp::resolver::results_type endpoints, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Resolve error: {}", error.message());
        return;
    }

    for (auto &endpoint : endpoints)
    {
        LOG_DEBUG_FMT("Endpoint: {}", endpoint.endpoint().address().to_string());
    }

    asio::async_connect(context->sock.lowest_layer(), endpoints, [context](const error_code &error, const tcp::endpoint &)
                        { onConnect(error, context); });
}

void HttpClient3::onConnect(const error_code &error, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Connect error: {}", error.message());
        return;
    }

    LOG_DEBUG("Client connected and handshaking..");

    context->sock.async_handshake(ssl::stream_base::client, [context](const error_code &error)
                                  { onHandshake(error, context); });
}

void HttpClient3::onHandshake(const error_code &error, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Handshake error: {}", error.message());
        return;
    }

    LOG_DEBUG("Client handshaked and writing..");
    auto &buffer = context->reqBuff;
    asio::async_write(context->sock, buffer, [context](const error_code &error, std::size_t bytes_transferred)
                      { onWrite(error, bytes_transferred, context); });
}

void HttpClient3::onWrite(const error_code &error, std::size_t bytes_transferred, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Write error: {}", error.message());
        return;
    }

    LOG_DEBUG_FMT("Client wrote {} bytes", bytes_transferred);

    auto &buffer = context->respBuff;

    // read http status line
    asio::async_read_until(context->sock, buffer, "\r\n",
                           [context](const error_code &error, std::size_t size)
                           {
                               onReadStatusLine(error, size, context);
                           });
}

void HttpClient3::onReadStatusLine(const error_code &error, const std::size_t read_size, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Read status line error: {}", error.message());
        return;
    }

    LOG_DEBUG_FMT("Read status line done. {} bytes", read_size);

    // copy and remove characters from the resp buf
    auto &buffer = context->respBuff;
    StatusLine statueLine(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + read_size);
    // context->respStatusLine = std::move(statueLine);
    // LOG_DEBUG_FMT("Status line: {}", context->respStatusLine);
    LOG_DEBUG_FMT("buffer size: len({})", buffer.size());
    buffer.consume(read_size);

    HttpContext::parseStatusLine(statueLine, context->response);

    // read the response headers
    asio::async_read_until(context->sock, buffer, "\r\n\r\n",
                           [context](const error_code &error, std::size_t size)
                           {
                               onReadHeaders(error, size, context);
                           });
}

void HttpClient3::onReadHeaders(const error_code &error, const std::size_t read_size, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Read http lint error: {}", error.message());
        return;
    }

    // read response headers.
    auto &buffer = context->respBuff;
    std::istream response_stream(&buffer);
    std::string headerLine;

    while (std::getline(response_stream, headerLine) && headerLine != "\r")
    {
        LOG_DEBUG_FMT("Header line: {}", headerLine);
        HttpContext::parseHeaderLine(headerLine, context->response->headers);
    }

    LOG_DEBUG_FMT("read header: len({})", read_size);
    LOG_DEBUG_FMT("buffer size: len({})", buffer.size());

    if (context->response->chunked())
    {
        readChunkSize(context);
    }
    else
    {
        asio::async_read(context->sock, buffer, boost::asio::transfer_at_least(1),
                         [context](const error_code &err, std::size_t size)
                         {
                             onReadIdentityBody(err, size, context);
                         });
    }
}

void HttpClient3::onReadIdentityBody(const error_code &error, const std::size_t read_size, const HttpContextPtr &context)
{
    if (error && error != asio::error::eof)
    {
        LOG_ERROR_FMT("Read body error: {}", error.message());
        context->complete(error);
        return;
    }

    auto &buffer = context->respBuff;
    RespBody body(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + buffer.size());
    buffer.consume(buffer.size());
    LOG_DEBUG_FMT("RespBody: {}", body);
    context->response->content.append(body);

    if (!error)
    {
        return asio::async_read(context->sock, buffer, boost::asio::transfer_at_least(1),
                                [context](const error_code &err, std::size_t size)
                                {
                                    onReadIdentityBody(err, size, context);
                                });
    }

    LOG_INFO("Read response done");
    context->complete(asio::error::eof);
}

void HttpClient3::readChunkSize(HttpContextPtr context)
{
    auto &buffer = context->respBuff;
    asio::async_read_until(context->sock, buffer, "\r\n",
                           [context](const error_code &error, std::size_t read_size)
                           {
                               if (error && error != asio::error::eof)
                               {
                                   LOG_ERROR_FMT("Read body error: {}", error.message());
                                   return context->complete(error);
                               }

                               auto &buffer = context->respBuff;
                               RespBody body(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + read_size);
                               buffer.consume(read_size);
                               size_t chunk_size = std::stoul(body, nullptr, 16);
                               LOG_DEBUG_FMT("Read chunk size: {}", chunk_size);

                               if (chunk_size == 0)
                               {
                                   LOG_INFO("Read chunk to end.");
                                   return context->complete(asio::error::eof);
                               }

                               readChunkBody(chunk_size, context);
                           });
}

void HttpClient3::readChunkBody(const std::size_t chunk_size, HttpContextPtr context)
{
    auto &buffer = context->respBuff;
    size_t to_read_size = chunk_size + 2; // content + "\r\n"
    asio::async_read(context->sock, buffer, asio::transfer_at_least(to_read_size),
                     [context, chunk_size](const error_code &error, std::size_t read_size)
                     {
                         if (error && error != asio::error::eof)
                         {
                             LOG_ERROR_FMT("Read body error: {}", error.message());
                             return context->complete(error);
                         }
                         auto &buffer = context->respBuff;
                         RespBody body(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + chunk_size);
                         LOG_DEBUG_FMT("read chunk: {}", body);

                         buffer.consume(chunk_size + 2);

                         context->response->content.append(body);
                         readChunkSize(context);
                     });
}