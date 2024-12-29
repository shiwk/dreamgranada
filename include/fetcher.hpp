#ifndef FETCHER_HPP
#define FETCHER_HPP

#include "roles.hpp"
#include "logger.hpp"
#include "clients.hpp"

namespace granada
{
    class Fetcher : public roles::GranadaRole
    {
    public:
        Fetcher(events::BusPtr bus) : roles::GranadaRole(bus) {};
        virtual void OnEvent(events::EventPtr event) override;
        ~Fetcher() {
            LOG_INFO( "Fetcher destroyed");
        }
    };
    
} // namespace granada

#endif