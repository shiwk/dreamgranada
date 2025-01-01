#ifndef OFFICE_HPP
#define OFFICE_HPP
#include "common.hpp"
#include "event.hpp"
#include "bus.hpp"
#include "roles.hpp"

namespace granada
{
    class PublishCenter : public events::BusStop
    {
    public:
        PublishCenter(events::BusPtr);
        virtual ~PublishCenter();
        virtual void onEvent(events::EventPtr) override;
        void subscribe(roles::GranadaRolePtr role);

    private:
        std::vector<roles::GranadaRolePtr> roles_;
    };
};

#endif