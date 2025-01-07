#ifndef ROLES_HPP
#define ROLES_HPP

#include "bus.hpp"
#include "clients3.hpp"
#include "logger.hpp"
#include "uuid.hpp"

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

        using EventHitMap = unsigned int;
        class GranadaRole : public Poster
        {
        public:
            GranadaRole(events::BusPtr bus, EventHitMap ehm, const uuid& id) : Poster(bus), ehm_(ehm), id_(id) {};
            virtual void onEvent(events::EventPtr event) = 0;
            void logIn(std::shared_ptr<PublishCenter>);
            virtual const uuid id() const;
            virtual ~GranadaRole() {}
            EventHitMap ehm() const;

            template<class T>
            static std::shared_ptr<T> instance(events::BusPtr bus, EventHitMap ehm, const std::string& prefix="")
            {
                auto &uuid_gen = granada::GranadaUID::instance();
                return std::make_shared<T>(bus, ehm, uuid_gen.gen(prefix));
            }
        private:
            EventHitMap ehm_;
            uuid id_;
        };

        MAKE_SHARED_PTR(GranadaRole);
    }
}

#endif