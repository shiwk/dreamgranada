

#include "roles.hpp"
#include "office.hpp"

namespace granada
{

    void roles::Poster::post(events::EventPtr event)
    {
        // A delayed handler can outlive its bus (e.g. at shutdown). Dropping
        // the event is safer than dereferencing an expired weak_ptr.
        auto bus = bus_.lock();
        if (!bus)
        {
            LOG_DEBUG_FMT("Dropping event {}: bus already destroyed", event->name());
            return;
        }
        bus->board(event);
    }

    void roles::Poster::asyncRequest3(http::RequestPtr &request, http::ResponseHandler &&respHandler, http::ErrorHandler &&errorHandler)
    {
        auto &context = events::Bus::getBusEngine();
        http::asyncRequest(context, request, std::move(respHandler), std::move(errorHandler));
        LOG_DEBUG("Request sent.");
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