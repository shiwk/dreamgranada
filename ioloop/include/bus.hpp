#ifndef IOLOOP_HPP
#define IOLOOP_HPP

#include <boost/asio.hpp>
#include "handler.hpp"
#include "event.hpp"
#include "common.hpp"
#include <boost/enable_shared_from_this.hpp>

using namespace boost;
using namespace boost::asio;
namespace granada
{
    namespace events
    {
        class Bus : public boost::enable_shared_from_this<Bus>
        {
            MAKE_SHARED_PTR(io_context);

        private:
            io_contextPtr io_context_;
            strand<io_context::executor_type> strand_;
            EventHandlerPtr event_handler_;
            std::future<void> io_thread_;
            std::function<void(EventPtr)> handlerCallback_;
            static void onTimeout(std::shared_ptr<asio::steady_timer> &, std::shared_ptr<io_context> &);

        public:
            Bus();
            ~Bus();
            void setHandlerCallback(std::function<void(EventPtr)>);
            void publish(EventPtr &);
            void start();
            io_context &getIOContext() const;
            io_contextPtr &getIOContextPtr();
        };

        MAKE_SHARED_PTR(Bus);
        MAKE_WEAK_PTR(Bus);
    }

}

#endif