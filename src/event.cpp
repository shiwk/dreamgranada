#include "event.hpp"
#include <string>

#define EVENT_SYSTEM_DESC_LENTH_BIT_COUNT 6
#define EVENT_SYSTEM_DESC_LENTH_BITS 0x3F // 0x00111111

using namespace granada::events;

event_desc granada::events::Event::desc() const
{
    return desc_;
}

event_desc granada::events::Event::usrDesc() const
{
    bitcout_t sysDescBitcount = static_cast<bitcout_t>(desc_ & EVENT_SYSTEM_DESC_LENTH_BITS);
    return desc_ >> sysDescBitcount;
}

event_desc granada::events::Event::sysDesc() const
{
    bitcout_t sysDescBitcount = static_cast<bitcout_t>(desc_ & EVENT_SYSTEM_DESC_LENTH_BITS);
    auto mask = (1 << sysDescBitcount) - 1;
    return desc_ & mask;
}

const delay_t granada::events::Event::delay() const
{
    delay_t d = static_cast<delay_t>(desc_ >> EVENT_SYSTEM_DESC_LENTH_BIT_COUNT);
    return d;
}

const active_t granada::events::Event::active() const
{
    return static_cast<active_t>(desc_ >> (EVENT_SYSTEM_DESC_LENTH_BIT_COUNT + sizeof(delay_t)));
}

const uint64_t granada::events::Event::ts() const
{
    return ts_;
}

event_desc granada::events::Event::sysInfo(delay_t delay, active_t active)
{
    /*
         |high bits for usr ########################################     low bits for system|
         |#####################################################|active(8)|delay(8)|syslen(6)|

         delay: delay in second before post
         active: event validity period
    */

    event_desc desc = active;
    desc = desc << sizeof(delay_t) * 8;
    desc = desc | delay;
    desc = desc << EVENT_SYSTEM_DESC_LENTH_BIT_COUNT;                                           // syslen
    desc = desc | (sizeof(active_t) + sizeof(delay_t)) * 8 + EVENT_SYSTEM_DESC_LENTH_BIT_COUNT; // ex: 8 + 8 + 6 = 24
    return desc;
}

event_desc granada::events::Event::concatenate(event_desc sys, event_desc usr)
{
    bitcout_t sysDescBitcount = static_cast<bitcout_t>(sys & EVENT_SYSTEM_DESC_LENTH_BITS);
    event_desc desc = (usr << sysDescBitcount) | sys;
    return desc;
}