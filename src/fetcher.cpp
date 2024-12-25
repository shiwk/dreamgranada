#include "fetcher.hpp"
#include "logger.hpp"

namespace granada
{
    void Fetcher::OnEvent(events::EventPtr event)
    {
        LOG_FMT(INFO, "Fetcher Received event {} {}", event->name(), event->type());
        asyncRequest(GET, "www.baidu.com", "/", [this](const boost::system::error_code &ec, std::string response)
                    {
                        LOG_FMT(INFO, "Fetcher received response: {}", response);
                        post(std::make_shared<events::CommonEvent>());
                    });
        LOG_MSG(INFO, "Fetcher sending request");
    }

}
