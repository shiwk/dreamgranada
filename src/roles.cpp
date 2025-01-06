

#include "roles.hpp"
#include "office.hpp"

namespace granada
{

    void roles::Poster::post(events::EventPtr event)
    {
        bus_.lock()->board(event);
    }

    void roles::Poster::asyncRequest3(http::RequestPtr & request, const http::ResponseHandler & respHandler, const http::ErrorHandler & errorHandler)
    {
        auto &context = events::Bus::getBusEngine();
        http::HttpClient3::asyncRequest(context, request, respHandler, errorHandler);
        LOG_INFO( "Request sent.");
    }

    void roles::GranadaRole::logIn(PublishCenterPtr publishCenter)
    {
        auto self = std::dynamic_pointer_cast<roles::GranadaRole> (shared_from_this());
        publishCenter->subscribe(self);
    }

    roles::event_hit_map roles::GranadaRole::ehm() const{
        return ehm_;
    }

    const ll roles::GranadaRole::id() const
    {
        return id_;
    }
}