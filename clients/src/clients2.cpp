#include "clients2.hpp"
#include "logger.hpp"

using namespace boost;
using namespace granada::http;

void HttpClient2::asyncRequest(RequestPtr &request, const ResponseHandler &requestCallback, const ErrorHandler &errorHandler)
{
    auto resolver = std::make_shared<tcp::resolver>(*io_context_);
    auto context = std::make_shared<HttpContext>(io_context_, request, requestCallback, errorHandler);
    resolver->async_resolve(request->host, request->https ? HTTPS : HTTP, [this, context](const error_code &err, tcp::resolver::results_type endpoints)
                            { onResolve(err, endpoints, context); });
}

void HttpClient2::onResolve(const error_code &error, tcp::resolver::results_type endpoints, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Resolve error: {}", error.message());
        return;
    }

    asio::async_connect(context->sock.lowest_layer(), endpoints, [this, context](const error_code &error, const tcp::endpoint &)
                        { onConnect(error, context); });
}

void HttpClient2::onConnect(const error_code &error, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Connect error: {}", error.message());
        return;
    }

    context->sock.async_handshake(ssl::stream_base::client, [this, context](const error_code &error)
                                  { onHandshake(error, context); });
}

void HttpClient2::onHandshake(const error_code &error, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Handshake error: {}", error.message());
        return;
    }

    auto &buffer = context->reqBuff;
    asio::async_write(context->sock, buffer, [this, context](const error_code &error, std::size_t bytes_transferred)
                      { onWrite(error, bytes_transferred, context); });
}

void HttpClient2::onWrite(const error_code &error, std::size_t bytes_transferred, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Write error: {}", error.message());
        return;
    }

    auto &buffer = context->respBuff;

    // read http line
    asio::async_read_until(context->sock, buffer, "\r\n",
                           [this, context](const error_code &error, std::size_t)
                           {
                               onReadStatusLine(error, context);
                           });
}

void HttpClient2::onReadStatusLine(const error_code &error, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Read status line error: {}", error.message());
        return;
    }

    // copy and remove characters from the resp buf
    auto &buffer = context->respBuff;
    StatusLine statueLine(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + buffer.size());
    context->respStatusLine = std::move(statueLine);
    buffer.consume(buffer.size());

    // read the response headers
    asio::async_read_until(context->sock, buffer, "\r\n\r\n",
                           [this, context](const error_code &error,
                                           std::size_t)
                           {
                               onReadHeaders(error, context);
                           });
}

void HttpClient2::onReadHeaders(const error_code &error, const HttpContextPtr &context)
{
    if (error)
    {
        LOG_ERROR_FMT("Read http lint error: {}", error.message());
        return;
    }

    // read response headers.
    std::istream response_stream(&context->respBuff);
    std::string headerLine;
    while (std::getline(response_stream, headerLine) && headerLine != "\r")
    {
        context->addRespHeaderLine(headerLine);
    }

    auto &buffer = context->respBuff;

    asio::async_read(context->sock, buffer, boost::asio::transfer_at_least(1),
                     [this, context](const error_code &err,
                                     std::size_t)
                     {
                         onReadBody(err, context);
                     });
}

void HttpClient2::onReadBody(const error_code &error, const HttpContextPtr &context)
{
    if (!error)
    {
        auto &buffer = context->respBuff;
        RespBody body(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + buffer.size());
        buffer.consume(buffer.size());
        context->respBody.append(body);

        asio::async_read(context->sock, buffer, boost::asio::transfer_at_least(1),
                                [this, context](const error_code &err, std::size_t)
                                {
                                    onReadBody(err, context);
                                });
    }

    context->complete(error);

    return;
}
