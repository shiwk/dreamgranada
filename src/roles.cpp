

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
        http::asyncRequest(context, request, respHandler, errorHandler);
        LOG_INFO("Request sent.");
    }

    void roles::Subscriber::logIn()
    {
        auto self = shared_T_from_this<roles::Subscriber>();
        post(std::make_shared<NewSubscriberLoginEvent>(id(), self));
    }

    bool roles::Subscriber::interest(events::event_desc usrDesc) const
    {
        return interest(ehm(), usrDesc);
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