#ifndef IOLOOP_HPP
#define IOLOOP_HPP

#include <boost/asio.hpp>
#include "handler.hpp"
#include "event.hpp"
#include "common.hpp"


using namespace boost;
namespace granada
{
    namespace events
    {
        class Bus
        {
        private:
            std::shared_ptr<asio::io_context> io_context_;
            asio::strand<asio::io_context::executor_type> strand_;
            EventHandlerPtr event_handler_;
            std::future<void> io_thread_;
            std::function<void(EventPtr)> handlerCallback_;
            static void onTimeout(asio::steady_timer&, std::shared_ptr<asio::io_context>);
        public:
            Bus();
            ~Bus();
            void setHandlerCallback(std::function<void(EventPtr)>);
            void postEvent(EventPtr);
            void start();
            // std::shared_ptr<asio::io_context> getIOContext() const;
            asio::io_context& getIOContext() const;
            // void delayPost(EventPtr, std::chrono::seconds);
            
        };

        MAKE_SHARED_PTR(Bus);
        MAKE_WEAK_PTR(Bus);
    }
    
}

#endif