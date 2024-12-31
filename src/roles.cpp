

#include "roles.hpp"

namespace granada
{

    void roles::Poster::post(events::EventPtr event)
    {
        bus_.lock()->publish(event);
    }
    
    std::shared_ptr<clients::HttpClient> roles::Poster::asyncRequest(const std::string &method, const std::string &host, const std::string &path, const clients::RequestCallback &callback)
    {
        boost::asio::io_context &context = bus_.lock()->getIOContext();

        auto client = std::make_shared<clients::HttpClient>(context, true);
        client->asyncRequest(method, host, path,
                             [this, client, callback](const boost::system::error_code &ec, std::string response)
                             {
                                 callback(ec, response);
                             });
        return client;
    }

    void roles::Poster::asyncRequest2(http::RequestPtr & request, const http::ResponseHandler & respHandler, const http::ErrorHandler & errorHandler)
    {
        auto &context = bus_.lock()->getIOContextPtr();
        auto client = std::make_shared<http::HttpClient2>(context);
        client->asyncRequest(request, respHandler, errorHandler);
        LOG_INFO( "Request sent.");
    }


    void roles::Poster::asyncRequest3(http::RequestPtr & request, const http::ResponseHandler & respHandler, const http::ErrorHandler & errorHandler)
    {
        auto &context = bus_.lock()->getIOContextPtr();
        http::HttpClient3::asyncRequest(context, request, respHandler, errorHandler);
        LOG_INFO( "Request sent.");
    }
}