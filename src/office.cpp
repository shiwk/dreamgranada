#include "office.hpp"
#include "logger.hpp"
#include "bus.hpp"
#include <boost/asio.hpp>
#include "roles.hpp"

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
        auto t = event->type();
        if (ehmToRole_.find(t) != ehmToRole_.end())
        {
            return;
        }

        for (auto iter = ehmToRole_[t].begin(); iter != ehmToRole_[t].end() ; iter++)
        {
            if (roles_.find(*iter) == roles_.end())
            {
                LOG_FATAL_FMT("role {} not subscribed", *iter);
            }

            roles_[*iter]->onEvent(event);
        }        
    }
    
    void PublishCenter::subscribe(roles::GranadaRolePtr role)
    {
        LOG_INFO( "Subscribing role");
        roles::event_hit_map ehm = role->ehm();
        size_t pos = 0;
        
        while (pos < EVENT_HIT_MAP_LENGTH)
        {
            if ((role->ehm() >> pos) & 0x01)
            {
                if(ehmToRole_.find(pos) == ehmToRole_.end())
                {
                    ehmToRole_[pos] = {role->id()};
                }
                else if (ehmToRole_[pos].find(role->id()) == ehmToRole_[pos].end())
                {
                    ehmToRole_[pos].insert(role->id());
                }
            }
            else
            {
                if(ehmToRole_.find(pos) != ehmToRole_.end() && ehmToRole_[pos].find(role->id()) != ehmToRole_[pos].end())
                {
                    ehmToRole_[pos].erase(role->id());
                }
            }
            pos += 1;
        }

        roles_[role->id()] = role;        
    }
}