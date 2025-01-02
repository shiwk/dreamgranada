

#include "roles.hpp"

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
}