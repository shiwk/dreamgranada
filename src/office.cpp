#include "office.hpp"
#include "logger.hpp"
#include "bus.hpp"
#include <boost/asio.hpp>
#include "roles.hpp"
#include <util.hpp>

using namespace granada::events;
#define OFFICE_SUBSCRIBER "OFFICE_SUBSCRIBER"
namespace granada
{
    OfficeCenter::OfficeCenter(events::BusPtr bus)
    {
        bus->setBusStop([this](const events::EventPtr &event)
                        { onEvent(event); });
    }

    void OfficeCenter::onEvent(events::EventPtr event)
    {
        if (event->usrDesc() == NewSubscriber)
        {
            onSubscribe(std::static_pointer_cast<NewSubscriberLoginEvent>(event));
        }
    }

    OfficeCenter::~OfficeCenter()
    {
        LOG_INFO("PublishCenter destroyed");
    }

    void OfficeCenter::onStop(events::EventPtr event)
    {
        LOG_DEBUG_FMT("Received event {} {}", event->name(), utils::Format::dumpB(event->desc()));
        
        event_desc desc = event->usrDesc();

        if (roles::Subscriber::interest(OFFICE_MASK, desc))
        {
            onEvent(event);
        }

        for (auto &subscriber : subscribers_)
        {
            if (subscriber->interest(desc))
            {
                subscriber->onEvent(event);
            }
        }
    }

    void OfficeCenter::onSubscribe(NewSubscriberLoginEventPtr event)
    {
        LOG_DEBUG_FMT("Subscribing role id = {}, ehm = {}", event->subscriber_->id(), utils::Format::dumpB(event->subscriber_->ehm()));
        subscribers_.emplace_back(event->subscriber_);
    }
    
    const std::string &NewSubscriberLoginEvent::name() const
    {
        static const std::string OFFICESUBSCRIBER = OFFICE_SUBSCRIBER;
        return OFFICESUBSCRIBER;
    }
}