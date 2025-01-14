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
            void asyncRequest3(http::RequestPtr &request, const http::ResponseHandler &respHandler, const http::ErrorHandler &errorHandler);

        protected:
            events::BusWeakPtr bus_;
        };

        using EventHitMap = unsigned long long;
        class GranadaRole : public Poster
        {
        public:
            GranadaRole(events::BusPtr bus, const uuid &id) : Poster(bus), id_(id) {};

            virtual const uuid id() const;
            virtual ~GranadaRole() {}

        private:
            uuid id_;
            virtual void handleError(granada::events::event_desc, const std::string &errorMsg) = 0;
        };

        MAKE_SHARED_PTR(GranadaRole);

        class Subscriber : public GranadaRole
        {
        private:
            EventHitMap ehm_;

        public:
            Subscriber(events::BusPtr bus, const uuid &id, EventHitMap ehm) : GranadaRole(bus, id), ehm_(ehm) {}
            virtual void onEvent(events::EventPtr event) = 0;
            void logIn(std::shared_ptr<PublishCenter>);
            EventHitMap ehm() const;
            template <class T>
            static std::shared_ptr<T> instance(events::BusPtr bus, const std::string &prefix, EventHitMap ehm)
            {
                auto &uuid_gen = granada::GranadaUID::instance();
                return std::make_shared<T>(bus, uuid_gen.gen(prefix), ehm);
            }

            template <class T>
            static std::shared_ptr<T> instance(events::BusPtr bus, const std::string &prefix)
            {
                auto &uuid_gen = granada::GranadaUID::instance();
                return std::make_shared<T>(bus, uuid_gen.gen(prefix));
            }

            inline static bool hit(EventHitMap ehm, events::event_desc usr_desc)
            {
                events::bitcout_t bitcount = static_cast<events::bitcout_t>(ehm & 0XFF);
                auto mask = (1 << bitcount) - 1;
                auto bitMap = ehm >> 8;
                return bitMap != 0  && (usr_desc & mask) == (bitMap & mask);
            }
        };
        MAKE_SHARED_PTR(Subscriber);

    }
}

#endif