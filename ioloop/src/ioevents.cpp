#include "ioevents.hpp"
#include "event_handler.hpp"

namespace granada
{
    namespace ioloop
    {
        granada::ioloop::ioevents::ioevents(EventHandlerPtr event_handler) : io_context_(std::make_unique<boost::asio::io_context>()),
                                                                             strand_(boost::asio::make_strand(*io_context_)),
                                                                             event_handler_(event_handler) {}

        granada::ioloop::ioevents::~ioevents()
        {
            io_context_->stop();
            io_thread_.wait();
        }

        void granada::ioloop::ioevents::run()
        {

            io_thread_ = std::async(std::launch::async, [this]
                                    { io_context_->run(); });
        }

        void granada::ioloop::ioevents::newEvent(EventPtr event)
        {
            boost::asio::post(strand_, [this, event](){ event_handler_->handle(event); });
        }
    }
}
