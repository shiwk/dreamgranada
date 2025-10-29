#include "reqresp.hpp"
#include <encode.hpp>
#include <logger.hpp>
#include <util.hpp>

granada::http::HttpStatus granada::http::status::OK = "200";
namespace http = granada::http;

granada::http::Request::Request(const Method &method, const std::string &host, const std::string &path, const std::string &user_agent, const std::string &connection, const std::string &body, const bool https)
    : method(method), path(path), host(host), user_agent(user_agent), body(body), connection(connection), https(https)
{
    headers[Connection] = connection;
    headers[User_Agent] = user_agent;
    headers[Content_Length] = std::to_string(body.size());
}

void granada::http::Request::addHeader(const std::string &header, const std::string &value)
{
    headers[header] = value;
}

void granada::http::Request::addQuery(const std::string &key, const std::string &value)
{
    queries[key] = value;
}

const std::string granada::http::methodToString(const http::Method &method)
{
    switch (method)
    {
    case Method::GET:
        return "GET";
    case Method::POST:
        return "POST";
    case Method::PUT:
        return "PUT";
    case Method::DELETE:
        return "DELETE";
    case Method::HEAD:
        return "HEAD";
    default:
        return "UNKNOWN";
    }
}

bool http::parseHeaderLine(const http::HeaderLine &line, http::ResponsePtr &response)
{
    size_t pos = line.find(':');
    if (pos == std::string::npos)
    {
        LOG_ERROR_FMT("Invalid header line: {}", line);
        return false;
    }

    std::string key = granada::utils::string::trim(std::move(line.substr(0, pos)));
    std::string value = granada::utils::string::trim(line.substr(pos + 1));

    LOG_DEBUG_FMT("header line {}:{}", key, value);
    response->headers[key] = value;

    return true;
}

bool http::parseStatusLine(const http::StatusLine &line, http::ResponsePtr &response)
{
    std::vector<std::string> tokens;
    std::istringstream tokenStream(line);

    std::string token;
    while (std::getline(tokenStream, token, ' '))
    {
        tokens.push_back(token);
    }

    if (tokens.size() < 3)
    {
        LOG_ERROR_FMT("Invalid status line: {}", line);
        return false;
    }
    std::istringstream stream(line);

    stream >> response->version;

    stream >> response->statusCode;

    std::getline(stream, response->statusMessage);

    if (!response->statusMessage.empty() && response->statusMessage[0] == ' ')
    {
        response->statusMessage.erase(0, 1);
    }
    return true;
}

void http::writeQueryStream(const http::RequestPtr &request, std::ostream &stream, bool sort)
{

    std::vector<std::pair<std::string, std::string>> params(request->queries.begin(), request->queries.end());

    if (sort)
    {
        std::sort(params.begin(), params.end(),
                  [](const auto &a, const auto &b)
                  {
                      return granada::utils::encode::urlEncode(a.first) < granada::utils::encode::urlEncode(b.first);
                  });
    }

    for (auto it = params.begin(); it != params.end(); ++it)
    {
        if (it != params.begin())
        {
            stream << "&";
        }
        stream << granada::utils::encode::urlEncode(it->first);
        if (!it->second.empty())
        {
            stream << "=" << granada::utils::encode::urlEncode(it->second);
        }
    }
}

bool granada::http::shouldReadBody(const http::ResponsePtr &response)
{
    try
    {
        if (response->version == HTTP_1_0)
        {
            // HTTP/1.0 always read body
            return true;
        }

        // check status code
        int code = std::stoi(response->statusCode);
        if ((100 <= code && code < 200) || code == 204 || code == 304)
            return false;

        // Transfer-Encoding: chunked
        if (response->chunked())
        {
            return true;
        }

        // Content-Length
        auto it = response->headers.find(Content_Length);
        it = it != response->headers.end() ? it : response->headers.find(granada::utils::string::toLower(Content_Length));
        if (it != response->headers.end())
        {
            size_t len = std::stoul(it->second);
            return len > 0;
        }

        it = response->headers.find(Connection);
        it = it != response->headers.end() ? it : response->headers.find(granada::utils::string::toLower(Connection));
        if (it != response->headers.end())
        {
            std::string conn = it->second;
            std::transform(conn.begin(), conn.end(), conn.begin(), ::tolower);
            if (conn.find("close") != std::string::npos)
                return true; // to old http1.0
        }

        // default
        return false;
    }
    catch (...)
    {
        LOG_ERROR("Failed to parse response.");
        return false;
    }
}
