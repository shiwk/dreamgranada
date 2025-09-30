#include "event.hpp"
#include <string>
#include <cstdint>
#include <util.hpp>

#define EVENT_SYSTEM_DESC_LENTH_BIT_COUNT 6
#define EVENT_SYSTEM_DESC_LENTH_BITS 0x3F // 0B00111111

using namespace granada::events;

event_desc granada::events::Event::desc() const
{
    return desc_;
}

event_desc granada::events::Event::usrDesc() const
{
    bitcout_t sysDescBitcount = sysDescBitCount();
    return desc_ >> sysDescBitcount;
}

event_desc granada::events::Event::sysDesc() const
{
    bitcout_t sysDescBitcount = sysDescBitCount();
    auto mask = (1 << sysDescBitcount) - 1;
    return desc_ & mask;
}

const delay_t granada::events::Event::delay() const
{
    bitcout_t sysDescBitcount = sysDescBitCount();


    event_desc desc = desc_ >> EVENT_SYSTEM_DESC_LENTH_BIT_COUNT;
    size_t delayBitsCount = sysDescBitcount - EVENT_SYSTEM_DESC_LENTH_BIT_COUNT;
    desc = desc & (1LL << delayBitsCount) - 1;
    return desc;
}

const granada::uuid granada::events::Event::poster() const
{
    return poster_;
}

const active_t granada::events::Event::active() const
{
    // return static_cast<active_t>(desc_ >> (EVENT_SYSTEM_DESC_LENTH_BIT_COUNT + sizeof(delay_t)));
    // todo: active is not used
    return 0xFF;
}

const uint64_t granada::events::Event::ts() const
{
    return ts_;
}

event_desc granada::events::Event::sysInfo(delay_t delay, active_t active)
{
    /*
         |high bits for usr ########################################     low bits for system|
         |#####################################################|active(8)|delay(16)|syslen(6)|

         delay: delay in second before post
         active: event validity period
    */

    event_desc desc = delay;
    desc = desc << EVENT_SYSTEM_DESC_LENTH_BIT_COUNT;
    size_t minBytesRequired = granada::utils::NumUtil::minBytesRequired(delay);
    size_t minBitsRequired = minBytesRequired * 8;

    desc = desc | minBitsRequired + EVENT_SYSTEM_DESC_LENTH_BIT_COUNT; // minBytesRequired(delay) * 8 + 6
    return desc;
}

event_desc granada::events::Event::concatenate(event_desc sys, event_desc usr)
{
    bitcout_t sysDescBitcount = static_cast<bitcout_t>(sys & EVENT_SYSTEM_DESC_LENTH_BITS);
    event_desc desc = (usr << sysDescBitcount) | sys;
    return desc;
}

const size_t granada::events::Event::sysDescBitCount() const
{
    bitcout_t sysDescBitcount = static_cast<bitcout_t>(desc_ & EVENT_SYSTEM_DESC_LENTH_BITS);
    return sysDescBitcount;
}