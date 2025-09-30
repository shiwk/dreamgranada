#include <event.hpp>
#include <util.hpp>
#include <gtest/gtest.h>
#include <uuid.hpp>
#include <iostream>

#define SUB_EVENT_BIT_LENGTH 8
#define USR_DESC(ROLE, EVENT) (((ROLE) << SUB_EVENT_BIT_LENGTH) | (EVENT))
namespace events = granada::events;

struct TestEvent : public events::DelayedEvent
{
    TestEvent(const granada::uuid &poster, events::delay_t delay, events::event_desc usr = 0) : DelayedEvent(poster, delay, usr) {}
    virtual const std::string &name() const override
    {
        static std::string TESTEVENT = "TESTEVENT";
        return TESTEVENT;
    };
};

TEST(AdditionTest, HandlesPositiveNumbers)
{
    {
        uint64_t d = 31036000;
        events::delay_t delay = d;
        EXPECT_EQ(4, granada::utils::NumUtil::minBytesRequired(delay));
        TestEvent event("id", delay, USR_DESC(0x15, 3));
        EXPECT_EQ(6+32, event.sysDescBitCount());
        EXPECT_EQ(delay, event.delay());
        std::cout << granada::utils::Format::dumpB(event.desc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.sysDesc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.usrDesc()) << std::endl;;
    }

    {
        uint32_t d = 31036000;
        events::delay_t delay = d;
        EXPECT_EQ(4, granada::utils::NumUtil::minBytesRequired(delay));
        TestEvent event("id", delay, USR_DESC(0x15, 3));
        EXPECT_EQ(6+32, event.sysDescBitCount());
        EXPECT_EQ(delay, event.delay());
        std::cout << granada::utils::Format::dumpB(event.desc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.sysDesc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.usrDesc()) << std::endl;;
    }
    
    {
        uint64_t d = 4294967295;
        events::delay_t delay = d;
        EXPECT_EQ(4, granada::utils::NumUtil::minBytesRequired(delay));
        TestEvent event("id", delay, USR_DESC(0x15, 3));
        EXPECT_EQ(6+32, event.sysDescBitCount());
        EXPECT_EQ(delay, event.delay());
        std::cout << granada::utils::Format::dumpB(event.desc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.sysDesc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.usrDesc()) << std::endl;;
    }

    {
        uint32_t d = 4294967295;
        events::delay_t delay = d;
        EXPECT_EQ(4, granada::utils::NumUtil::minBytesRequired(delay));
        TestEvent event("id", delay, USR_DESC(0x15, 3));
        EXPECT_EQ(6+32, event.sysDescBitCount());
        EXPECT_EQ(delay, event.delay());
        std::cout << granada::utils::Format::dumpB(event.desc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.sysDesc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.usrDesc()) << std::endl;;
    }
    
    {
        uint64_t d = 1;
        events::delay_t delay = d;
        EXPECT_EQ(1, granada::utils::NumUtil::minBytesRequired(delay));
        TestEvent event("id", delay, USR_DESC(0x15, 3));
        EXPECT_EQ(6+8, event.sysDescBitCount());
        EXPECT_EQ(delay, event.delay());
        std::cout << granada::utils::Format::dumpB(event.desc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.sysDesc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.usrDesc()) << std::endl;;
    }

    {
        uint32_t d = 1;
        events::delay_t delay = d;
        EXPECT_EQ(1, granada::utils::NumUtil::minBytesRequired(delay));
        TestEvent event("id", delay, USR_DESC(0x15, 3));
        EXPECT_EQ(6+8, event.sysDescBitCount());
        EXPECT_EQ(delay, event.delay());
        std::cout << granada::utils::Format::dumpB(event.desc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.sysDesc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.usrDesc()) << std::endl;;
    }
    
    {
        uint32_t d = 0;
        events::delay_t delay = d;
        EXPECT_EQ(1, granada::utils::NumUtil::minBytesRequired(delay));
        TestEvent event("id", delay, USR_DESC(0x15, 3));
        EXPECT_EQ(6+8, event.sysDescBitCount());
        EXPECT_EQ(delay, event.delay());
        std::cout << granada::utils::Format::dumpB(event.desc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.sysDesc()) << std::endl;;
        std::cout << granada::utils::Format::dumpB(event.usrDesc()) << std::endl;;
    }
}