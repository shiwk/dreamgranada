#ifndef ROLES_HPP
#define ROLES_HPP

#include "bus.hpp"
#include "clients3.hpp"
#include "logger.hpp"
#include "uuid.hpp"
#include "event.hpp"

#define MASK(c) ((1LL << c) - 1)
#define ROLE_MASK_BIT_LENGTH 8
#define GRANADA_EVENT_BIT_LENGTH 8
#define GRANADA_USR_DESC(ROLE, EVENT) (((ROLE) << GRANADA_EVENT_BIT_LENGTH) | (EVENT)) // 8bits for rolemask, 8bits for event


namespace http = granada::http;
namespace granada
{
    class OfficeCenter;
    namespace roles
    {
        using role_desc = uint8_t;
        class Poster
        {
        public:
            Poster(events::BusPtr bus) : bus_(bus) {}
            virtual ~Poster() {}
            void post(events::EventPtr event);
            void asyncRequest3(http::RequestPtr &request, http::ResponseHandler &&, http::ErrorHandler &&);

        protected:
            events::BusWeakPtr bus_;
        };

        using EventHitMap = unsigned long long;
        class GranadaRole : public Poster,  public std::enable_shared_from_this<GranadaRole>
        {
        public:
            GranadaRole(events::BusPtr bus, const uuid &id) : Poster(bus), id_(id) {};

            virtual const uuid id() const;
            virtual ~GranadaRole() {}
        protected:
            template<class T>
            std::shared_ptr<T> shared_T_from_this()
            {
                #ifdef DEBUG_BUILD
                auto ptr = std::dynamic_pointer_cast<T>(this->shared_from_this());
                assert(ptr && "Wrong type passed to shared_T_from_this!");
                return ptr;
                #else
                return std::static_pointer_cast<T>(this->shared_from_this());
                #endif
            }

        private:
            uuid id_;
        };

        MAKE_SHARED_PTR(GranadaRole);

        class Subscriber : public GranadaRole
        {
        private:
            EventHitMap ehm_;

        public:
            Subscriber(events::BusPtr bus, const uuid &id, EventHitMap ehm) : GranadaRole(bus, id), ehm_(ehm) {}
            virtual void onEvent(events::EventPtr event) = 0;
            virtual bool interest(events::event_desc usrDesc) const;
            void logIn();
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
                auto ins = std::make_shared<T>(bus, uuid_gen.gen(prefix));
                ins->logIn();
                return ins;
            }

            inline static bool interest(role_desc ehm, events::event_desc usrDesc)
            {
                return ehm != 0  && (usrDesc >> GRANADA_EVENT_BIT_LENGTH) == ehm;
            }
        };
        MAKE_SHARED_PTR(Subscriber);
    }
}

#endif