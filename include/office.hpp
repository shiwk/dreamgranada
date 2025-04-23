#ifndef OFFICE_HPP
#define OFFICE_HPP
#include "common.hpp"
#include "event.hpp"
#include "bus.hpp"
#include "roles.hpp"
#include <set>

namespace granada
{
    const events::event_desc OFFICE_MASK= 0x01;
    
    enum OfficeEventDesc
    {
        NewSubscriber =  COMBINE(OFFICE_MASK, 0x00)
    };

    struct NewSubscriberLoginEvent : public events::CommonEvent
    {
        NewSubscriberLoginEvent(const uuid &poster, roles::SubscriberPtr subscriber) : events::CommonEvent(poster, OfficeEventDesc::NewSubscriber), subscriber_(subscriber) {}
        roles::SubscriberPtr subscriber_;
        virtual const std::string &name() const override;
    };

    MAKE_SHARED_PTR(NewSubscriberLoginEvent);

    class OfficeCenter : public events::BusStop
    {
    public:
        OfficeCenter(events::BusPtr);
        void onEvent(events::EventPtr);
        virtual ~OfficeCenter();
        virtual void onStop(events::EventPtr) override;

    private:
        void onSubscribe(NewSubscriberLoginEventPtr);
        std::vector<roles::SubscriberPtr> subscribers_;
    };
    MAKE_SHARED_PTR(OfficeCenter);
};

#endif