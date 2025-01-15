

#include "roles.hpp"
#include "office.hpp"

namespace granada
{

    void roles::Poster::post(events::EventPtr event)
    {
        bus_.lock()->board(event);
    }

    void roles::Poster::asyncRequest3(http::RequestPtr &request, const http::ResponseHandler &respHandler, const http::ErrorHandler &errorHandler)
    {
        auto &context = events::Bus::getBusEngine();
        http::HttpClient3::asyncRequest(context, request, respHandler, errorHandler);
        LOG_INFO("Request sent.");
    }

    void roles::Subscriber::logIn()
    {
        auto self = std::dynamic_pointer_cast<roles::Subscriber>(shared_from_this());
        post(std::make_shared<NewSubscriberLoginEvent>(id(), self));
    }

    roles::EventHitMap roles::Subscriber::ehm() const
    {
        return ehm_;
    }

    const std::string roles::GranadaRole::id() const
    {
        return id_;
    }
}