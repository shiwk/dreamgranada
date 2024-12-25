

#include "roles.hpp"

namespace granada
{

    void roles::Post::post(events::EventPtr event)
    {
        bus_.lock()->postEvent(event);
    }
    std::shared_ptr<clients::HttpClient> roles::Post::asyncRequest(const std::string &method, const std::string &host, const std::string &path, const clients::RequestCallback &callback)
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
}