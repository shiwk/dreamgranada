#ifndef OFFICE_HPP
#define OFFICE_HPP
#include "common.hpp"
#include "event.hpp"
#include "bus.hpp"
#include "roles.hpp"
#include <set>

namespace granada
{
    class PublishCenter : public events::BusStop
    {
    public:
        PublishCenter(events::BusPtr);
        virtual ~PublishCenter();
        virtual void OnEvent(events::EventPtr) override;
        void subscribe(roles::GranadaRolePtr role);

    private:
        std::unordered_map<std::string, roles::GranadaRolePtr> roles_;
        std::unordered_map<roles::EventHitMap, std::set<std::string>> ehmToRole_;
    };
    MAKE_SHARED_PTR(PublishCenter);
};

#endif