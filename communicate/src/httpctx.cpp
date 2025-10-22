#include "httpctx.hpp"
#include "logger.hpp"
#include <openssl/err.h>
#include <encode.hpp>
#include <util.hpp>

namespace http = granada::http;

ssl::context &http::getSSLCtx()
{
    static asio::ssl::context ctx(asio::ssl::context::tls_client);
    static bool ctxInit = []
    {
        ctx.set_options(
            asio::ssl::context::default_workarounds |
            asio::ssl::context::no_sslv2 |
            asio::ssl::context::no_sslv3 |
            asio::ssl::context::no_tlsv1 |
            asio::ssl::context::no_tlsv1_1);
        ctx.set_default_verify_paths();
        LOG_DEBUG("ssl context initialized");
        return true;
    }();

    return ctx;
}

void http::HttpBasicContext::prepare(const http::RequestPtr &request)
{
    std::ostream reqStream(&reqBuff);
    reqStream << http::methodToString(request->method) << " " << request->path << (request->queries.empty() ? "" : "?");
    // write queries
    http::writeQueryStream(request, reqStream);

    reqStream << " HTTP/1.1\r\n";
    reqStream << "Host: " << request->host << "\r\n";

    // write headers
    for (auto it = request->headers.begin(); it != request->headers.end(); ++it)
    {
        reqStream << it->first << ": " << it->second << "\r\n";
    }

    reqStream << "\r\n";

    // write body
    switch (request->method)
    {
    case Method::PUT:
    case Method::POST:
        reqStream << request->body;
        break;

    default:
        break;
    }

#ifdef DEBUG_BUILD
    auto bufs = reqBuff.data();
    std::string content(asio::buffers_begin(bufs), asio::buffers_end(bufs));
    LOG_DEBUG_FMT("Request stream:\n{}", content);
#endif

    timeout(request->timeout);
}

void http::safeCloseSsl(std::shared_ptr<sSock> &sock)
{
    boost::system::error_code ignored_ec;
    if (sock->lowest_layer().is_open())
    {
        sock->lowest_layer().shutdown(tcp::socket::shutdown_both, ignored_ec);
        sock->lowest_layer().close(ignored_ec);
        LOG_DEBUG("Socket safe closed.");
    }
}

void http::HttpBasicContext::timeout(uint64_t second)
{
    timer_->expires_after(std::chrono::seconds(second));
    auto ctx = shared_from_this();
    timer_->async_wait(
        [ctx](const error_code &ec)
        {
            if (ec == asio::error::operation_aborted)
            {
                // canceled
                LOG_ERROR_FMT("httpctx timeout: {}", ec.message());
            }
            ctx->cleanUp();
        });
}

void http::HttpBasicContext::dumpRequest()
{
    std::istream is(&reqBuff);
    std::string data((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    LOG_DEBUG_FMT("Request stream:\n{}", data);
    reqBuff.consume(reqBuff.size());
    is.clear();
    is.seekg(0);
}

template <>
void http::HttpContext<http::sSock>::cleanUp()
{
    std::string remote_endpoint;
    try
    {
        if (!sock->lowest_layer().is_open())
        {
            // already closed
            LOG_DEBUG("Socket already closed.");
            return;
        }
        remote_endpoint = sock->lowest_layer().remote_endpoint().address().to_string();
    }
    catch (const std::exception &e)
    {
        LOG_WARNING_FMT("Failed to get remote endpoint: {}", e.what());
        remote_endpoint = "unknown";
    }
    LOG_DEBUG_FMT("Starting SSL shutdown to {}", remote_endpoint);
    auto timer = std::make_shared<boost::asio::steady_timer>(*io_context_);
    // timeout
    timer->expires_after(std::chrono::seconds(5));
    auto ctx = std::static_pointer_cast<HttpContext<http::sSock>>(shared_from_this());

    sock->async_shutdown(
        [ctx, timer](const boost::system::error_code &ec)
        {
            // cancel timer
            timer->cancel();
            if (!ec)
            {
                LOG_DEBUG("SSL stream shutdown succeed.");
                return;
            }

            std::string sslErrString(256, 0);

            unsigned long err = ERR_get_error();
            if (err != 0)
            {
                ERR_error_string_n(err, &sslErrString[0], 256);
            }
            const std::string errMsg = ec.message();
            if (ec == asio::error::operation_aborted)
            {
                LOG_INFO("Shutdown cancelled (operation_aborted). Treat as closed or user-cancelled.");
                return;
            }
            if (ec.value() == EBADF ||
                ec == boost::system::error_code(static_cast<int>(boost::system::errc::bad_file_descriptor), boost::system::generic_category()))
            {
                LOG_WARNING("Socket already closed (bad_file_descriptor).");
                return http::safeCloseSsl(ctx->sock);
            }
            if (ec == boost::asio::ssl::error::stream_truncated)
            {
                LOG_INFO("Stream truncated during SSL shutdown; treating as normal close.");
                return http::safeCloseSsl(ctx->sock);
            }
            if (errMsg.find("application data after close notify") != std::string::npos ||
                sslErrString.find("application data after close notify") != std::string::npos)
            {
                LOG_WARNING("Application data after close_notify detected. Treating as closed.");
                return http::safeCloseSsl(ctx->sock);
            }
            if (errMsg.find("shutdown while in init") != std::string::npos ||
                sslErrString.find("shutdown while in init") != std::string::npos)
            {
                LOG_ERROR("Called SSL_shutdown while SSL in init/handshake state. Programming error: ensure handshake finished before shutdown.");
                return http::safeCloseSsl(ctx->sock);
            }
            if (errMsg.find("uninitialized") != std::string::npos ||
                sslErrString.find("uninitialized") != std::string::npos)
            {
                LOG_ERROR("SSL_shutdown reported 'uninitialized'. Likely using freed/uninitialized SSL object. Fix lifecycle management.");
                return http::safeCloseSsl(ctx->sock);
            }

            LOG_ERROR_FMT("SSL shutdown error: {}, OpenSSL error: {}", errMsg, sslErrString);
            http::safeCloseSsl(ctx->sock);
        });

    timer->async_wait([ctx](const boost::system::error_code &ec)
                      {
        if (!ec) {
            LOG_DEBUG("Shutdown timed out, closing socket");
            return http::safeCloseSsl(ctx->sock);
        } });
}

template <>
void http::HttpContext<http::tSock>::cleanUp()
{
    if (!sock->is_open())
    {
        LOG_DEBUG("Socket already closed.");
        return;
    }
    LOG_DEBUG("Closing socket.");
    sock->shutdown(tcp::socket::shutdown_both);
    sock->close();
}

template <>
http::HttpContext<tcp::socket>::HttpContext(io_contextPtr &io_context, const http::RequestPtr &request, http::ResponseHandler &&respHandler, http::ErrorHandler &&errorHandler)
    : HttpBasicContext(io_context, request, std::move(respHandler), std::move(errorHandler)), sock(std::make_shared<tcp::socket>(*io_context))
{
}

template <>
http::HttpContext<http::sSock>::HttpContext(io_contextPtr &io_context, const http::RequestPtr &request, http::ResponseHandler &&respHandler, http::ErrorHandler &&errorHandler)
    : HttpBasicContext(io_context, request, std::move(respHandler), std::move(errorHandler)), sock(std::make_shared<http::sSock>(*io_context, getSSLCtx()))
{
}

http::HttpBasicContext::HttpBasicContext(io_contextPtr &io_context, const http::RequestPtr &request, http::ResponseHandler &&respHandler, http::ErrorHandler &&errorHandler)
    : io_context_(io_context), reqBuff(), respBuff(), respHandler(std::move(respHandler)), errorHandler(std::move(errorHandler)), timer_(std::make_shared<asio::steady_timer>(*io_context))
{
}

http::HttpBasicContext::~HttpBasicContext()
{
    LOG_DEBUG("HttpBasicContext destroyed.");
}

void http::HttpBasicContext::complete(const error_code &error, ResponsePtr response)
{
    timer_->cancel();
    if (error != boost::asio::error::eof)
    {
        LOG_ERROR("Httpctx error occured.");
        errorHandler(error);
        // cleanUp();
        return;
    }

    respHandler(error, response);
    // cleanUp();
}
