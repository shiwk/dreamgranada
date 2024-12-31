#include "fetcher.hpp"
#include "logger.hpp"

namespace granada
{
    void Fetcher::OnEvent(events::EventPtr event)
    {
        LOG_INFO_FMT("Fetcher Received event {} {}", event->name(), event->type());
        auto request = std::make_shared<http::Request>(http::Method::GET, "www.baidu.com", "/");
        asyncRequest3(request, [this](const boost::system::error_code &ec, http::ResponsePtr &response)
                    {
                        LOG_INFO_FMT("Fetcher received response: {}", response->content);
                        post(std::make_shared<events::DelayedEvent>(1));
                    },
                    [this](const boost::system::error_code &ec)
                    {
                        LOG_ERROR_FMT("Fetcher error: {}", ec.message());
                    });
        LOG_INFO( "Fetcher sending request");
        // sleep(10);
    }
}
