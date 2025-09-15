#include "httpctx.hpp"
#include "logger.hpp"
#include <openssl/err.h>
#include <encode.hpp>

using namespace granada::http;

Request::Request(const Method &method, const std::string &host, const std::string &path, const std::string &user_agent, const std::string &connection, const std::string &body, bool https)
    : https(https), method(method), path(path), host(host), user_agent(user_agent), body(body), connection(connection)
{
}

void Request::addHeader(const std::string &header, const std::string &value)
{
    headers[header] = value;
}

void Request::addQuery(const std::string &key, const std::string &value)
{
    queries[key] = value;
}

void HttpContext::writeQueryStream(std::unordered_map<std::string, std::string> &queries, std::ostream &stream)
{
    for (auto it = queries.begin(); it != queries.end(); ++it)
    {
        if (it != queries.begin())
        {
            stream << "&";
        }
        stream << granada::urlEncode(it->first) << "=" << granada::urlEncode(it->second);
    }
}

void HttpContext::writeResqHeaders(std::unordered_map<std::string, std::string> &headers, std::ostream &stream)
{
    for (auto it = headers.begin(); it != headers.end(); ++it)
    {
        stream << it->first << ": " << it->second << "\r\n";
    }
}

ssl::context &HttpContext::getSSLCtx()
{
    static asio::ssl::context ctx(asio::ssl::context::tlsv13);
    return ctx;
}

void HttpContext::prepareRequest(const RequestPtr &request)
{
    if (SSL_set_tlsext_host_name(sock.native_handle(), request->host.c_str()) != 1) {
        LOG_ERROR("Failed to set SNI hostname.");
    }

    std::ostream reqStream(&reqBuff);
    std::ostringstream queryStream;
    size_t contentLength = 0;
    std::string queryStr;
    switch (request->method)
    {
    case Method::GET:
        reqStream << "GET " << request->path << (request->queries.empty() ? "" : "?");
        writeQueryStream(request->queries, reqStream);
        reqStream << " HTTP/1.1\r\n";
        reqStream << "Host: " << request->host << "\r\n";
        writeResqHeaders(request->headers, reqStream);
        reqStream << "User-Agent: " << request->user_agent << "\r\n";
        reqStream << "Connection: " << request->connection << "\r\n";
        reqStream << "\r\n";

        break;

    case Method::POST:
        reqStream << "POST " << request->path << (request->queries.empty() ? "" : "?");
        writeQueryStream(request->queries, reqStream);
        reqStream << " HTTP/1.1\r\n";
        reqStream << "Host: " << request->host << "\r\n";
        writeResqHeaders(request->headers, reqStream);
        reqStream << "Content-Length: " << request->body.size() << "\r\n";
        reqStream << "User-Agent: " << request->user_agent << "\r\n";
        reqStream << "Connection: " << request->connection << "\r\n";
        reqStream << "\r\n";
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
}

void HttpContext::complete(const error_code &error)
{
    if (error != boost::asio::error::eof)
    {
        LOG_ERROR("Httpctx error occured.");
        errorHandler(error);
        cleanUp();
        return;
    }

    respHandler(error, response);
    cleanUp();
}

bool HttpContext::parseStatusLine(const StatusLine &line, ResponseStatusPtr resp)
{
    std::vector<std::string> tokens;
    HttpContextUtil::split(line, ' ', tokens);

    if (tokens.size() < 3)
    {
        LOG_ERROR_FMT("Invalid status line: {}", line);
        for (const auto &token : tokens)
        {
            LOG_ERROR_FMT("Token: {}", token);
        }
        return false;
    }
    std::istringstream stream(line);

    stream >> resp->version;

    stream >> resp->statusCode;

    std::getline(stream, resp->statusMessage);

    if (!resp->statusMessage.empty() && resp->statusMessage[0] == ' ')
    {
        resp->statusMessage.erase(0, 1);
    }
    return true;
}

std::shared_ptr<asio::steady_timer> granada::http::HttpContext::getTimer()
{
    std::shared_ptr<asio::steady_timer> ctxTimer = std::make_shared<asio::steady_timer>(*io_context_, std::chrono::seconds(timeout_));
    auto ctx = shared_from_this();
    ctxTimer->async_wait([ctx, ctxTimer](const error_code &ec)
                                          {
                                            if (!ec)
                                            {
                                                LOG_ERROR_FMT("httpctx timeout. {}, timeout: {}", ec.message(), ctx->timeout_);
                                                ctx->cleanUp();
                                            }
                                            ctxTimer->cancel();
                                        });
    return ctxTimer;
}

bool HttpContext::parseHeaders(const std::vector<HeaderLine> &lines, RespHeaders &headers)
{
    for (const auto &line : lines)
    {
        if (!parseHeaderLine(line, headers))
        {
            return false;
        }
    }

    return true;
}

bool granada::http::HttpContext::parseHeaderLine(const HeaderLine &line, RespHeaders &headers)
{
    size_t pos = line.find(':');
    if (pos == std::string::npos)
    {
        LOG_ERROR_FMT("Invalid header line: {}", line);
        return false;
    }

    std::string key = HttpContextUtil::strip(std::move(line.substr(0, pos)));
    std::string value = HttpContextUtil::strip(line.substr(pos + 1));
    headers[key] = value;
    // LOG_DEBUG_FMT("header {}:{}", key, value);

    return true;
}

void HttpContextUtil::split(const std::string &s, const char delimiter, std::vector<std::string> &tokens)
{
    std::string token;
    std::istringstream tokenStream(s);

    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
}

std::string granada::http::HttpContextUtil::strip(const std::string &str)
{
    auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
    auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();

    return (start < end) ? std::move(std::string(start, end)) : "";
}

void granada::http::HttpContext::dumpRequest(RequestPtr request)
{
    std::istream is(&reqBuff);
    std::string data((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    LOG_DEBUG_FMT("Request stream: {}", data);
    reqBuff.consume(reqBuff.size());
    is.clear();
    is.seekg(0);
}

void granada::http::HttpContext::safeClose()
{
    boost::system::error_code ignored_ec;
    if (sock.lowest_layer().is_open())
    {
        sock.lowest_layer().shutdown(tcp::socket::shutdown_both, ignored_ec);
        sock.lowest_layer().close(ignored_ec);
        LOG_DEBUG("Socket safe closed.");
    }
}

HttpContext::~HttpContext()
{
    // cleanUp();
    LOG_DEBUG("HttpContext destroyed.");
}

void HttpContext::cleanUp()
{
    std::string remote_endpoint;
    try
    {
        if (!sock.lowest_layer().is_open())
        {
            // already closed
            LOG_DEBUG("Socket already closed.");
            return;
        }
        remote_endpoint = sock.lowest_layer().remote_endpoint().address().to_string();
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
    auto ctx = shared_from_this();
    sock.async_shutdown(
        [ctx, timer](const boost::system::error_code& ec) {
            // cancel timer
            timer->cancel();
            if (!ec)
            {
                LOG_DEBUG("SSL stream shutdown succeed.");
                return;
            }
            
            std::string sslErrString(256, 0);

            unsigned long err = ERR_get_error();
            if (err != 0){
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
                return ctx->safeClose();
            }
            if (ec == boost::asio::ssl::error::stream_truncated) {
                LOG_INFO("Stream truncated during SSL shutdown; treating as normal close.");
                return ctx->safeClose();
            }
            if (errMsg.find("application data after close notify") != std::string::npos ||
                sslErrString.find("application data after close notify") != std::string::npos)
            {
                LOG_WARNING("Application data after close_notify detected. Treating as closed.");
                return ctx->safeClose();
            }
            if (errMsg.find("shutdown while in init") != std::string::npos ||
                sslErrString.find("shutdown while in init") != std::string::npos)
            {
                LOG_ERROR("Called SSL_shutdown while SSL in init/handshake state. Programming error: ensure handshake finished before shutdown.");
                return ctx->safeClose();
            }
            if (errMsg.find("uninitialized") != std::string::npos ||
                sslErrString.find("uninitialized") != std::string::npos)
            {
                LOG_ERROR("SSL_shutdown reported 'uninitialized'. Likely using freed/uninitialized SSL object. Fix lifecycle management.");
                return ctx->safeClose();
            }
            
            LOG_ERROR_FMT("SSL shutdown error: {}, OpenSSL error: {}", errMsg, sslErrString);
            ctx->safeClose();
        });

    timer->async_wait([ctx](const boost::system::error_code& ec) {
        if (!ec) {
            LOG_DEBUG("Shutdown timed out, closing socket");
            return ctx->safeClose();
        }
    });
}
