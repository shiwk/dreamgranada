#include "clients.hpp"
#include "logger.hpp"

namespace granada
{
    namespace clients
    {


        asio::ssl::context &HttpClient::getClientSSLCtx()
        {
            static asio::ssl::context ctx(asio::ssl::context::sslv23);
            ctx.set_default_verify_paths();
            return ctx;
        }

        void HttpClient::asyncRequest(const std::string &method, const std::string &host, const std::string &path, const RequestCallback &requestCallback)
        {
            host_ = host;
            path_ = path;
            method_ = method;
            requestCallback_ = requestCallback;
            LOG_INFO_FMT("Requesting {} {}://{}", method, https_ ? HTTPS : HTTP, host + path);
            resolver_.async_resolve(host, https_ ? HTTPS : HTTP, [this](const system::error_code &error, tcp::resolver::results_type endpoints)
                                    { onResolve(error, endpoints); });
        }

        void HttpClient::onResolve(const system::error_code &error, tcp::resolver::results_type endpoints)
        {
            LOG_INFO( "Client resloved and connecting..");
            if (error)
            {
                LOG_ERROR_FMT("Resolve error: {}", error.message());
                return;
            }
            for (auto &endpoint : endpoints)
            {
                LOG_INFO_FMT("Endpoint: {}", endpoint.endpoint().address().to_string());
            }
            asio::async_connect(socket_.lowest_layer(), endpoints, [this](const system::error_code &error, tcp::endpoint)
                                { onConnect(error); });
        }

        void HttpClient::onConnect(const system::error_code &error)
        {
            LOG_INFO( "Client connecting..");

            if (error)
            {
                LOG_ERROR_FMT("Connect error: {}", error.message());
                return;
            }

            socket_.async_handshake(ssl::stream_base::client, [this](const system::error_code &error)
                                    { onHandshake(error); });
        }

        void HttpClient::onHandshake(const system::error_code &error)
        {
            LOG_INFO( "Client handshaked");

            if (error)
            {
                LOG_ERROR_FMT("Handshake error: {}", error.message());
                return;
            }

            std::string request = httpMessage(method_, host_, path_);
            asio::async_write(socket_, asio::buffer(request), [this](const system::error_code &error, std::size_t bytes_transferred)
                              { onWrite(error, bytes_transferred); });
        }

        void HttpClient::onWrite(const system::error_code &error, std::size_t bytes_transferred)
        {
            LOG_INFO( "Client writing..");

            if (error)
            {
                LOG_ERROR_FMT("Write error: {}", error.message());
                return;
            }

            asio::async_read(socket_, asio::buffer(buffer_), [this](const system::error_code &error, std::size_t bytes_transferred)
                             { onRead(error, bytes_transferred); });
        }

        void HttpClient::onRead(const system::error_code &error, std::size_t bytes_transferred)
        {
            LOG_INFO( "Client reading..");

            if (error && error != asio::error::eof)
            {
                LOG_ERROR_FMT("Read error: {}", error.message());
                return;
            }

            const std::string response(buffer_.data(), bytes_transferred);
            LOG_INFO_FMT("Response:\n{}", response);
            requestCallback_(error, response);

            socket_.lowest_layer().close();
        }

        std::string HttpClient::httpMessage(const std::string &method,
                                            const std::string &path,
                                            const std::string &host,
                                            const std::string &user_agent,
                                            const std::string &connection,
                                            const std::string &body)
        {
            std::ostringstream request;
            request << method << " " << path << " HTTP/1.1\r\n";
            request << "Host: " << host << "\r\n";
            request << "User-Agent: " << user_agent << "\r\n";
            request << "Connection: " << connection << "\r\n";
            request << "Content-Length: " << body.size() << "\r\n";
            request << "\r\n";
            request << body;
            return request.str();
        }

        HttpClient::~HttpClient()
        {
            LOG_INFO( "HttpClient destroyed");
            if (socket_.lowest_layer().is_open())
            {
                socket_.lowest_layer().close();
            }
        }

        HttpClient::HttpClient(asio::io_context &io_context, bool https):
              resolver_(io_context),
              socket_(io_context, getClientSSLCtx()),
              https_(https)
        {
            LOG_INFO( "Client created");
        }
    }
};