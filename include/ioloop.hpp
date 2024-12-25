#ifndef WORKER_HPP
#define WORKER_HPP
#include "common.hpp"
#include "handler.hpp"
#include "event.hpp"
#include "bus.hpp"
#include "roles.hpp"

namespace granada
{
    class IOLoop : public events::EventHandler
    {
    public:
        IOLoop(events::BusPtr);
        virtual ~IOLoop();
        virtual void handle(events::EventPtr) override;
        void subscribe(roles::GranadaRolePtr role);
    private:
        std::vector<roles::GranadaRolePtr> roles_;
    };

    // MAKE_SHARED_PTR(Worker);

};

#endif