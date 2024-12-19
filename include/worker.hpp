#ifndef WORKER_HPP
#define WORKER_HPP
#include "common.hpp"
#include "event_handler.hpp"
#include "event.hpp"

namespace granada
{
    class Worker : public ioloop::EventHandler
    {
    public:
        Worker();
        virtual ~Worker();
        virtual void handle(ioloop::EventPtr) override;
    };

    // MAKE_SHARED_PTR(Worker);

};

#endif