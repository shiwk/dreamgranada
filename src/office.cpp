#include "office.hpp"
#include "logger.hpp"
#include "bus.hpp"
#include <boost/asio.hpp>
#include "roles.hpp"
#include <util.hpp>

using namespace granada::events;

namespace granada
{
    PublishCenter::PublishCenter(events::BusPtr bus)
    {
        bus->setBusStop([this](const events::EventPtr &event)
                        { onEvent(event); });
    }

    PublishCenter::~PublishCenter()
    {
        LOG_INFO("PublishCenter destroyed");
    }

    void PublishCenter::onEvent(events::EventPtr event)
    {
        LOG_DEBUG_FMT("Received event {} {}", event->name(), utils::Format::dumpB(event->desc()));
        event_desc desc = event->usrDesc();

        for (auto & subscriber : subscribers_)
        {
            if (roles::Subscriber::hit(subscriber->ehm(), desc))
            {
                subscriber->onEvent(event);
            }
        }
    }

    void PublishCenter::subscribe(roles::SubscriberPtr subscriber)
    {
        LOG_DEBUG_FMT("Subscribing role id = {}, ehm = {}", subscriber->id(), utils::Format::dumpB(subscriber->ehm()));
        subscribers_.emplace_back(subscriber);
    }
}