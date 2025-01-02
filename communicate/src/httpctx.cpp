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

#ifdef DEBUG_BUILD
// dumpRequest(request);
#endif
        break;

    case Method::POST:
        reqStream << "POST " << request->path << " HTTP/1.1\r\n";
        reqStream << "Host: " << request->host << "\r\n";
        reqStream << "User-Agent: " << request->user_agent << "\r\n";
        reqStream << "Accept: */*\r\n";
        reqStream << "Content-Type: application/x-www-form-urlencoded\r\n";
        writeQueryStream(request->queries, queryStream);
        queryStr = queryStream.str();
        contentLength = queryStr.size();
        reqStream << "Content-Length: " << contentLength << "\r\n";
        reqStream << "Connection: close\r\n";
        reqStream << "\r\n";
        reqStream << queryStr;
        break;

    default:
        break;
    }
}

void HttpContext::addRespHeaderLine(const std::string &line)
{
    respHeaders.push_back(line);
}

void HttpContext::complete(const error_code &error)
{
    if (error != boost::asio::error::eof)
    {
        LOG_ERROR("Read body error");
        errorHandler(error);
        return;
    }

    ResponsePtr resp = std::make_shared<Response>();
    bool success = getResponse(resp);
    if (!success)
    {
        LOG_ERROR("Failed to parse response");
        errorHandler(error);
        return;
    }

    respHandler(error, resp);
}

bool HttpContext::getResponse(ResponsePtr &resp)
{
    bool parseResult = false;
    parseResult = parseStatusLine(respStatusLine, *resp);
    if (!parseResult)
    {
        LOG_ERROR("Failed to parse status line");
        return false;
    }
    parseResult = parseHeaders(respHeaders, resp->headers);
    if (!parseResult)
    {
        LOG_ERROR("Failed to parse headers");
        return false;
    }

    resp->content = std::move(respBody);
    return true;
}

bool HttpContext::parseStatusLine(const StatusLine &line, ResponseStatus &resp)
{
    std::vector<std::string> tokens;
    split(line, ' ', tokens);

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

    stream >> resp.version;

    stream >> resp.statusCode;

    std::getline(stream, resp.statusMessage);

    if (!resp.statusMessage.empty() && resp.statusMessage[0] == ' ')
    {
        resp.statusMessage.erase(0, 1);
    }
    return true;
}

bool HttpContext::parseHeaders(const std::vector<HeaderLine> &lines, RespHeaders &headers)
{
    for (const auto &line : lines)
    {
        size_t pos = line.find(':');
        if (pos == std::string::npos)
        {
            LOG_ERROR_FMT("Invalid header line: {}", line);
            return false;
        }

        std::string key = std::move(line.substr(0, pos));

        key = key.substr(key.find_last_not_of(" \t\n"), key.find_last_not_of(" \t\n") - key.find_first_not_of(" \t\n") + 1);
        std::string value = std::move(line.substr(pos + 1));
        value = value.substr(key.find_last_not_of(" \t\n"), value.find_last_not_of(" \t\n") - value.find_first_not_of(" \t\n") + 1);
        headers[key] = value;
    }

    return true;
}

void HttpContext::split(const std::string &s, const char delimiter, std::vector<std::string> &tokens)
{
    std::string token;
    std::istringstream tokenStream(s);

    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
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
    LOG_DEBUG("HttpContext destroyed");
    if (sock.lowest_layer().is_open())
    {
        sock.lowest_layer().close();
    }
}