#ifndef EVENT_HANLER_HPP
#define EVENT_HANLER_HPP

#include "event.hpp"
#include "common.hpp"

namespace granada
{
    namespace events
    {
        class EventHandler
        {
        public:
            virtual void handle(EventPtr) = 0;
            virtual ~EventHandler() {}
        };

        MAKE_SHARED_PTR(EventHandler);
    }
}

#endif