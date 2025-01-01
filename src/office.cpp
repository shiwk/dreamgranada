#include "office.hpp"
#include "logger.hpp"
#include "bus.hpp"
#include <boost/asio.hpp>

namespace granada
{
    PublishCenter::PublishCenter(events::BusPtr bus)
    {
        bus->setBusStop([this](const events::EventPtr &event)
                                { onEvent(event); });
    }

    PublishCenter::~PublishCenter()
    {
        LOG_INFO( "PublishCenter destroyed");
    }

    void PublishCenter::onEvent(events::EventPtr event)
    {
        LOG_INFO_FMT("Received event {} {}", event->name(), event->type());
        for (auto role : roles_)
        {
            role->OnEvent(event);
        }
    }
    
    void PublishCenter::subscribe(roles::GranadaRolePtr role)
    {
        LOG_INFO( "Subscribing role");
        roles_.push_back(role);
    }
}