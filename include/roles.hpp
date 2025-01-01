#ifndef ROLES_HPP
#define ROLES_HPP

#include "bus.hpp"
#include "clients3.hpp"
#include "logger.hpp"

namespace http = granada::http;
namespace granada
{
    namespace roles
    {
        class Poster
        {
        public:
            Poster(events::BusPtr bus) : bus_(bus) {}
            virtual ~Poster() {}
            void post(events::EventPtr event);
            void asyncRequest3(http::RequestPtr& request, const http::ResponseHandler &respHandler, const http::ErrorHandler &errorHandler);

        protected:
            events::BusWeakPtr bus_;
        };

        class GranadaRole : public Poster
        {
        public:
            GranadaRole(events::BusPtr bus) : Poster(bus) {};
            virtual void OnEvent(events::EventPtr event) = 0;
            virtual ~GranadaRole() {}

        private:
        };

        MAKE_SHARED_PTR(GranadaRole);
    }
}

#endif