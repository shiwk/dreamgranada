#include "httpctx.hpp"
#include "logger.hpp"

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
        stream << it->first << "=" << it->second;
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
    static asio::ssl::context ctx(asio::ssl::context::sslv23);
    return ctx;
}

void HttpContext::prepareRequest(const RequestPtr &request)
{
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
    LOG_DEBUG_FMT("Request stream: {}", content);
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

HttpContext::~HttpContext()
{
    // cleanUp();
    LOG_DEBUG("HttpContext destroyed.");
}

void HttpContext::cleanUp()
{
    // if (sock.lowest_layer().is_open())
    // {
    //     error_code error;
    //     error = sock.shutdown(error);
    //     LOG_DEBUG_FMT("tcp::socket shutdown: {}", error.message());
    //     sock.lowest_layer().close();
    // }

    auto timer = std::make_shared<boost::asio::steady_timer>(*io_context_);

    // timeout 
    timer->expires_after(std::chrono::seconds(5));

    sock.async_shutdown(
        [&, timer](const boost::system::error_code& ec) {
            // cancel timer
            timer->cancel();
            if (ec) {
                LOG_WARNING_FMT("Shutdown failed: {}", ec.message());
            } else {
                LOG_DEBUG("SSL stream shutdown.");
            }
        });

    timer->async_wait([&](const boost::system::error_code& ec) {
        if (!ec) {
            LOG_DEBUG("Shutdown timed out, closing socket.");
            boost::system::error_code ignore_ec;
            sock.lowest_layer().close(ignore_ec);
        }
    });
}