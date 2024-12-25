#include "event.hpp"
#include <string>

namespace granada
{
    events::event_type events::CommonEvent::type()
    {
        return 1;
    }

    const std::string &events::CommonEvent::name() const
    {
        static const std::string str = "COMMON";
        return str;
    }
}
