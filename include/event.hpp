#ifndef EVENT_HPP
#define EVENT_HPP

#include "common.hpp"
#include <chrono>
#include "uuid.hpp"
#include "ts.hpp"
#include <climits>

using namespace std::chrono;
#define EVENT_ID_PREFIX "E"

namespace granada
{
    namespace events
    {
        using event_desc = int64_t;
        using delay_t = uint16_t;
        using active_t = uint8_t;
        using bitcout_t = uint8_t;

        class Event
        {
        private:
            uint64_t ts_;
            uuid poster_;
            event_desc desc_;

            static event_desc sysInfo(delay_t delay, active_t period);
            static event_desc concatenate(event_desc sys, event_desc usr);

        public:
            Event(const uuid &poster, delay_t delay, active_t period, event_desc usr)
                : desc_(concatenate(sysInfo(delay, period), usr)),
                  ts_(GranadaTimestamp::getCurrentTimestamp()),
                  poster_(poster) {}

            virtual ~Event() {}
            event_desc desc() const;
            event_desc usrDesc() const;
            event_desc sysDesc() const;
            const delay_t delay() const;
            const active_t active() const;
            const uint64_t ts() const;
            virtual const std::string &name() const = 0;
        };
        MAKE_SHARED_PTR(Event);

        class CommonEvent : public Event
        {
        public:
            CommonEvent(const uuid &poster, event_desc usr = 0) : Event(poster, 0, 0xFF, usr) {}
            virtual ~CommonEvent() {}
        };

        class DelayedEvent : public Event
        {
        public:
            DelayedEvent(const uuid &poster, delay_t delay, event_desc usr = 0) : Event(poster, delay, 0xFF, usr) {}
            virtual ~DelayedEvent() {}
        };

        class ActiveEvent : public Event
        {
        public:
            ActiveEvent(const uuid &poster, active_t active, event_desc usr = 0) : Event(poster, 0, active, usr) {}
            virtual ~ActiveEvent() {}
        };

        class BusStop
        {
        public:
            virtual void onStop(EventPtr) = 0;
            virtual ~BusStop() {}
        };

        MAKE_SHARED_PTR(BusStop);
    }
}

#endif