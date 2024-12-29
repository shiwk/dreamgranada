#include "ioloop.hpp"
#include "logger.hpp"
#include "bus.hpp"
#include <boost/asio.hpp>

namespace granada
{
    IOLoop::IOLoop(events::BusPtr bus)
    {
        bus->setHandlerCallback([this](const events::EventPtr &event)
                                { handle(event); });
    }

    IOLoop::~IOLoop()
    {
        LOG_INFO( "IOLoop destroyed");
    }

    void IOLoop::handle(events::EventPtr event)
    {
        LOG_INFO_FMT("Received event {} {}", event->name(), event->type());
        for (auto role : roles_)
        {
            role->OnEvent(event);
        }
    }
    void IOLoop::subscribe(roles::GranadaRolePtr role)
    {
        LOG_INFO( "Subscribing role");
        roles_.push_back(role);
    }
}