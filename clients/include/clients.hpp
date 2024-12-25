#ifndef CLIENTS_HPP
#define CLIENTS_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/enable_shared_from_this.hpp>


using namespace boost;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

#define HTTP "http"
#define HTTPS "https"
#define GET "GET"

namespace granada
{
    namespace clients
    {
        using RequestCallback = std::function<void(const boost::system::error_code &, std::string)>;
        class HttpClient : public boost::enable_shared_from_this<HttpClient>
        {
        private:
            // std::shared_ptr<asio::io_context> io_context_;
            tcp::resolver resolver_;
            std::string host_;
            std::string path_;
            std::string method_;
            RequestCallback requestCallback_;
            bool https_;
            ssl::stream<tcp::socket> socket_;
            std::array<char, 1024> buffer_;
            static asio::ssl::context& getClientSSLCtx();
            static std::string httpMessage(const std::string &method,
                                           const std::string &path,
                                           const std::string &host,
                                           const std::string &user_agent = "granda_client",
                                           const std::string &connection = "close",
                                           const std::string &body = "");

        private:
            void onResolve(const system::error_code &error, tcp::resolver::results_type endpoints);
            void onConnect(const system::error_code &error);
            void onHandshake(const system::error_code &error);
            void onWrite(const system::error_code &error, std::size_t bytes_transferred);
            void onRead(const system::error_code &error, std::size_t bytes_transferred);

        public:
            // HttpClient(std::shared_ptr<asio::io_context> io_context, asio::ssl::context &ssl_context, bool https, const RequestCallback &clientCallback)
            // HttpClient(asio::io_context& io_context, asio::ssl::context &ssl_context, bool https, const ClientCallback &clientCallback)
            HttpClient(asio::io_context& io_context, bool https);
            ~HttpClient();
            void asyncRequest(const std::string &method, const std::string &host, const std::string &path, const RequestCallback &clientCallback);
        };

    }
}

#endif