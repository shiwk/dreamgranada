#ifndef EVENTS_HPP
#define EVENTS_HPP

#include "event.hpp"

namespace granada
{
    class CommonEvent : public ioloop::Event
    {
    public:
        virtual ioloop::event_type type() override;
        virtual ~CommonEvent() {}
    };
}

#endif