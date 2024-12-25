#ifndef EVENT_HPP
#define EVENT_HPP

#include "common.hpp"

namespace granada
{
    namespace events
    {
        using event_type = unsigned int;

        class Event
        {
        public:
            virtual event_type type() = 0;
            virtual const std::string &name() const = 0;
            virtual ~Event() {}
        };
        MAKE_SHARED_PTR(Event);

        class CommonEvent : public Event
        {
        public:
            virtual events::event_type type() override;
            virtual const std::string &name() const override;
            virtual ~CommonEvent() {}
        };
    }
}

#endif