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
        using event_desc = uint64_t;
        using delay_t = uint32_t;
        using active_t = uint8_t;
        using bitcout_t = uint8_t;

        struct EventPayload
        {
        };
        MAKE_SHARED_PTR(EventPayload);
        class Event
        {
        private:
            uint64_t ts_;
            uuid poster_;
            event_desc desc_;

            static event_desc sysInfo(delay_t delay, active_t period);
            static event_desc concatenate(event_desc sys, event_desc usr);
            template<class num>
            static size_t minBytesRequired(num value)
            {
                if (value == 0) return 1;

                int bits = sizeof(num) * 8 - __builtin_clz(value);
                return (bits + 7) / 8;
            }
        public:
            Event(const uuid &poster, delay_t delay, active_t period, event_desc usr)
                : desc_(concatenate(sysInfo(delay, period), usr)),
                  ts_(GranadaTimestamp::getCurrentTimestamp()),
                  poster_(poster) {}

            Event(const uuid &poster, delay_t delay, active_t period, event_desc usr, EventPayloadPtr payload)
                : desc_(concatenate(sysInfo(delay, period), usr)),
                  ts_(GranadaTimestamp::getCurrentTimestamp()),
                  poster_(poster),
                  payload(payload) {}

            virtual ~Event() {}
            event_desc desc() const;
            event_desc usrDesc() const;
            event_desc sysDesc() const;
            const delay_t delay() const;
            const active_t active() const;
            const uuid poster() const;
            const uint64_t ts() const;
            const size_t sysDescBitCount() const;
            virtual const std::string &name() const = 0;
            EventPayloadPtr payload;
        };
        MAKE_SHARED_PTR(Event);

        struct CommonEvent : public Event
        {
        public:
            CommonEvent(const uuid &poster, event_desc usr = 0) : Event(poster, 0, 0xFF, usr) {}
            CommonEvent(const uuid &poster, event_desc usr, EventPayloadPtr payload) : Event(poster, 0, 0xFF, usr, payload) {}
            virtual ~CommonEvent() {}
        };
        struct DelayedEvent : public Event
        {
            DelayedEvent(const uuid &poster, delay_t delay, event_desc usr = 0) : Event(poster, delay, 0xFF, usr) {}
            DelayedEvent(const uuid &poster, delay_t delay, event_desc usr, EventPayloadPtr payload) : Event(poster, delay, 0xFF, usr, payload) {}
            virtual ~DelayedEvent() {}
        };

        struct ActiveEvent : public Event
        {
            ActiveEvent(const uuid &poster, active_t active, event_desc usr = 0) : Event(poster, 0, active, usr) {}
            ActiveEvent(const uuid &poster, active_t active, event_desc usr, EventPayloadPtr payload) : Event(poster, 0, active, usr, payload) {}
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