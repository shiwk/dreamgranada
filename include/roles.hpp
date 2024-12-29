#ifndef ROLES_HPP
#define ROLES_HPP

#include "bus.hpp"
#include "clients.hpp"
#include "clients2.hpp"
#include "clients3.hpp"
#include "logger.hpp"

namespace http = granada::http;
namespace granada
{
    namespace roles
    {
        class Post
        {
        public:
            Post(events::BusPtr bus) : bus_(bus) {}
            virtual ~Post() {}
            void post(events::EventPtr event);
            std::shared_ptr<clients::HttpClient> asyncRequest(const std::string& method, const std::string &host, const std::string &path, const clients::RequestCallback &callback);
            void asyncRequest2(http::RequestPtr& request, const http::ResponseHandler &respHandler, const http::ErrorHandler &errorHandler);
            void asyncRequest3(http::RequestPtr& request, const http::ResponseHandler &respHandler, const http::ErrorHandler &errorHandler);

        protected:
            events::BusWeakPtr bus_;
        };

        class GranadaRole : public Post
        {
        public:
            GranadaRole(events::BusPtr bus) : Post(bus) {};
            virtual void OnEvent(events::EventPtr event) = 0;
            virtual ~GranadaRole() {}

        private:
        };

        MAKE_SHARED_PTR(GranadaRole);
    }
}

#endif