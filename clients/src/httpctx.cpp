#include "httpctx.hpp"
#include "logger.hpp"

using namespace granada::http;

void split(const std::string &s, char delimiter, std::vector<std::string> &tokens)
{
    std::string token;
    std::istringstream tokenStream(s);

    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
}

Request::Request(const Method &method, const std::string &path, const std::string &host, const std::string &user_agent, const std::string &connection, const std::string &body, bool https)
    : https(https), method(method), path(path), host(host), user_agent(user_agent), body(body)
{
    if (!body.empty())
    {
        contentLength = body.size();
    }
}

void Request::addHeader(const std::string &header, const std::string &value)
{
    headers[header] = value;
}

void Request::addQuery(const std::string &key, const std::string &value)
{
    queries[key] = value;
}

ssl::context &HttpContext::getSSLCtx()
{
    static asio::ssl::context ctx(asio::ssl::context::sslv23);
    return ctx;
}

void HttpContext::addRespHeaderLine(const std::string &line)
{
    respHeaders.push_back(line);
}

void HttpContext::complete(const error_code &error)
{
    if (error != boost::asio::error::eof)
    {
        LOG_MSG(ERROR, "Read body error");
        errorHandler(error);
        return;
    }

    LOG_MSG(INFO, "Read response body done");
    ResponsePtr resp = std::make_shared<Response>();
    bool success = getResponse(resp);
    if (!success)
    {
        LOG_MSG(ERROR, "Failed to parse response");
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
        LOG_MSG(ERROR, "Failed to parse status line");
        return false;
    }
    parseResult = parseHeaders(respHeaders, resp->headers);
    if (!parseResult)
    {
        LOG_MSG(ERROR, "Failed to parse headers");
        return false;
    }
    
    resp->content = std::move(respBody);
    return true;
}

bool HttpContext::parseStatusLine(const StatusLine &line, ResponseStatus & resp)
{
    std::vector<std::string> tokens;
    split(line, ' ', tokens);

    if (tokens.size() != 3)
    {
        LOG_FMT(ERROR, "Invalid status line: {}", line);
        return false;
    }
    resp.version = tokens[0];
    resp.statusCode = std::stoi(tokens[1]);
    resp.statusMessage = tokens[2];
    return true;
}

bool HttpContext::parseHeaders(const std::vector<HeaderLine> &lines, RespHeaders &headers)
{
    for (const auto &line : lines)
    {
        std::vector<std::string> tokens;
        split(line, ':', tokens);
        if (tokens.size() != 2)
        {
            LOG_FMT(ERROR, "Invalid header line: {}", line);
            return false;
        }
        std::string key = tokens[0];

        key = key.substr(key.find_last_not_of(" \t\n"), key.find_last_not_of(" \t\n") - key.find_first_not_of(" \t\n") + 1);
        std::string value = tokens[1];
        value = value.substr(key.find_last_not_of(" \t\n"), value.find_last_not_of(" \t\n") - value.find_first_not_of(" \t\n") + 1);
        headers[key] = value;
    }

    return true;
}
