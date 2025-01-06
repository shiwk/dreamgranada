#include "event.hpp"
#include <string>

namespace granada
{
    events::event_type events::CommonEvent::type()
    {
        return 0;
    }

    const std::string &events::CommonEvent::name() const
    {
        static const std::string str = "COMMON";
        return str;
    }

    events::event_type granada::events::DelayedEvent::type()
    {
        return 1;
    }

    const std::string &events::DelayedEvent::name() const
    {
        static const std::string str = "DELAYED";
        return str;
    }
}

const ll granada::events::Event::delay() const
{
    return delay_;
}

const time_point<system_clock> &granada::events::Event::ts() const
{
    return ts_;
}
