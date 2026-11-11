#include <gtest/gtest.h>
#include <office.hpp>
#include <roles.hpp>

#include <stdexcept>

namespace
{
    using granada::events::BusPtr;
    using granada::events::CommonEvent;
    using granada::events::EventPtr;

    // Role 0x02 so the base Subscriber::interest matches (usrDesc >> 8).
    const granada::roles::EventHitMap kTestRole = 0x02;
    const granada::events::event_desc kRecordingEvent = GRANADA_USR_DESC(0x02, 0x01);

    struct RecordingEvent : CommonEvent
    {
        explicit RecordingEvent(const granada::uuid &poster) : CommonEvent(poster, kRecordingEvent) {}
        const std::string &name() const override
        {
            static const std::string name = "RECORDING_EVENT";
            return name;
        }
    };

    class ThrowingSubscriber : public granada::roles::Subscriber
    {
    public:
        ThrowingSubscriber(BusPtr bus, const granada::uuid &id, bool nonStandard)
            : Subscriber(bus, id, kTestRole), nonStandard_(nonStandard) {}

        void onEvent(EventPtr) override
        {
            if (nonStandard_)
            {
                throw 42; // Not derived from std::exception.
            }
            throw std::runtime_error("subscriber failure");
        }

    private:
        bool nonStandard_;
    };

    class CountingSubscriber : public granada::roles::Subscriber
    {
    public:
        CountingSubscriber(BusPtr bus, const granada::uuid &id)
            : Subscriber(bus, id, kTestRole) {}

        void onEvent(EventPtr) override { ++count; }

        int count = 0;
    };
}

TEST(OfficeCenterTests, SubscriberExceptionsAreIsolated)
{
    auto bus = std::make_shared<granada::events::Bus>();
    auto officeCenter = std::make_shared<granada::OfficeCenter>(bus);

    auto throwingStd = std::make_shared<ThrowingSubscriber>(bus, "throwing_std", false);
    auto healthy = std::make_shared<CountingSubscriber>(bus, "healthy");
    auto throwingOther = std::make_shared<ThrowingSubscriber>(bus, "throwing_other", true);

    officeCenter->onStop(std::make_shared<granada::NewSubscriberLoginEvent>("tester", throwingStd));
    officeCenter->onStop(std::make_shared<granada::NewSubscriberLoginEvent>("tester", healthy));
    officeCenter->onStop(std::make_shared<granada::NewSubscriberLoginEvent>("tester", throwingOther));

    auto event = std::make_shared<RecordingEvent>("tester");

    // A throwing subscriber must neither escape the switchboard nor stop
    // delivery to the subscribers registered after it.
    EXPECT_NO_THROW(officeCenter->onStop(event));
    EXPECT_EQ(1, healthy->count);

    EXPECT_NO_THROW(officeCenter->onStop(event));
    EXPECT_EQ(2, healthy->count);
}

TEST(OfficeCenterTests, SubscriberWithoutInterestIsNotCalled)
{
    auto bus = std::make_shared<granada::events::Bus>();
    auto officeCenter = std::make_shared<granada::OfficeCenter>(bus);
    auto healthy = std::make_shared<CountingSubscriber>(bus, "healthy");

    officeCenter->onStop(std::make_shared<granada::NewSubscriberLoginEvent>("tester", healthy));

    // Role 0x03 does not match the subscriber's role 0x02.
    struct OtherRoleEvent : CommonEvent
    {
        explicit OtherRoleEvent(const granada::uuid &poster)
            : CommonEvent(poster, GRANADA_USR_DESC(0x03, 0x01)) {}
        const std::string &name() const override
        {
            static const std::string name = "OTHER_ROLE_EVENT";
            return name;
        }
    };
    officeCenter->onStop(std::make_shared<OtherRoleEvent>("tester"));

    EXPECT_EQ(0, healthy->count);
}