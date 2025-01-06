#ifndef ROLES_HPP
#define ROLES_HPP

#include "bus.hpp"
#include "clients3.hpp"
#include "logger.hpp"

#define EVENT_HIT_MAP_LENGTH 32

namespace http = granada::http;
namespace granada
{
    class PublishCenter;
    namespace roles
    {
        class Poster : public std::enable_shared_from_this<Poster>
        {
        public:
            Poster(events::BusPtr bus) : bus_(bus) {}
            virtual ~Poster() {}
            void post(events::EventPtr event);
            void asyncRequest3(http::RequestPtr& request, const http::ResponseHandler &respHandler, const http::ErrorHandler &errorHandler);

        protected:
            events::BusWeakPtr bus_;
        };

        using event_hit_map = unsigned int;
        class GranadaRole : public Poster
        {
        public:
            GranadaRole(events::BusPtr bus, event_hit_map ehm, ll id) : Poster(bus), ehm_(ehm), id_(id) {};
            virtual void onEvent(events::EventPtr event) = 0;
            void logIn(std::shared_ptr<PublishCenter>);
            virtual const ll id() const;
            virtual ~GranadaRole() {}
            event_hit_map ehm() const;
        private:
            event_hit_map ehm_;
            ll id_;
        };

        MAKE_SHARED_PTR(GranadaRole);
    }
}

#endif