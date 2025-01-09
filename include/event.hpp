#ifndef EVENT_HPP
#define EVENT_HPP

#include "common.hpp"
#include <chrono>

using namespace std::chrono;

namespace granada
{
    namespace events
    {
        using event_type = unsigned int;

        class Event
        {
        public:
            Event(ll delay) : delay_(delay), ts_(system_clock::now()) {}
            virtual event_type type() = 0;
            virtual const std::string &name() const = 0;
            virtual ~Event() {}
            const ll delay() const;
            const time_point<system_clock> &ts() const;

        private:
            ll delay_;
            time_point<system_clock> ts_;
        };
        MAKE_SHARED_PTR(Event);

        class CommonEvent : public Event
        {
        public:
            CommonEvent() : Event(0) {}
            virtual events::event_type type() override;
            virtual const std::string &name() const override;
            virtual ~CommonEvent() {}
        };

        class DelayedEvent : public Event
        {
        public:
            DelayedEvent(ll delay) : Event(delay) {}
            virtual events::event_type type() override;
            virtual const std::string &name() const override;
            virtual ~DelayedEvent() {}
        };

        class BusStop
        {
        public:
            virtual void OnEvent(EventPtr) = 0;
            virtual ~BusStop() {}
        };

        MAKE_SHARED_PTR(BusStop);
    }
}

#endif