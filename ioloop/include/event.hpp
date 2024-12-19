#ifndef EVENT_HPP
#define EVENT_HPP

#include "common.hpp"


namespace granada
{
    namespace ioloop
    {
        using event_type = unsigned int;

        class Event
        {
        public:
            virtual event_type type() = 0;
            virtual ~Event() {}
        };
        MAKE_SHARED_PTR(Event);
    }
}

#endif