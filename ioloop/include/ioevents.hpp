#ifndef IOLOOP_HPP
#define IOLOOP_HPP

#include <boost/asio.hpp>
#include "event_handler.hpp"
#include "event.hpp"

namespace granada
{
    namespace ioloop
    {
        class ioevents
        {
        private:
            std::unique_ptr<boost::asio::io_context> io_context_;
            boost::asio::strand<boost::asio::io_context::executor_type> strand_;
            EventHandlerPtr event_handler_;
            std::future<void> io_thread_;
        public:
            ioevents(EventHandlerPtr);
            ~ioevents();
            void newEvent(EventPtr);
            void run();
        };

    
    }
    
}

#endif