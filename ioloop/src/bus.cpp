#include "bus.hpp"
#include "handler.hpp"
#include "logger.hpp"

namespace granada
{
    namespace events
    {
        void Bus::onTimeout(std::shared_ptr<asio::steady_timer> &timer, std::shared_ptr<asio::io_context> &io)
        {
            timer->expires_at(timer->expiry() + std::chrono::seconds(1));
            timer->async_wait([&](const system::error_code &error)
                              {
                                  if (!error)
                                  {
                                    LOG_INFO("BUS HB ONCE");
                                  }
                                  else
                                  {
                                    LOG_ERROR_FMT("BUS ERROR: {}", error.message());
                                  }
                                  onTimeout(timer, io); });
        }

        Bus::Bus() : io_context_(std::make_shared<boost::asio::io_context>()),
                     strand_(boost::asio::make_strand(*io_context_)) {}

        Bus::~Bus()
        {
            LOG_INFO("Bus destructor");
            io_context_->stop();
            io_thread_.wait();
        }

        void Bus::setHandlerCallback(std::function<void(EventPtr)> handlerCallback)
        {
            handlerCallback_ = handlerCallback;
        }

        void Bus::start()
        {
            LOG_INFO("Bus starting..");
            std::shared_ptr<asio::steady_timer> timer = std::make_shared<asio::steady_timer>(*io_context_, std::chrono::seconds(1));
            onTimeout(timer, io_context_);
            io_context_->run();
        }

        asio::io_context &Bus::getIOContext() const
        {
            return *io_context_;
        }

        Bus::io_contextPtr &Bus::getIOContextPtr()
        {
            return io_context_;
        }

        void Bus::publish(EventPtr &event)
        {
            if (event->delay() > 0)
            {
                LOG_DEBUG_FMT("delayed event: {} delayed {}s ", event->name(), event->delay());
                std::shared_ptr<asio::steady_timer> timer = std::make_shared<asio::steady_timer>(*io_context_, std::chrono::seconds(event->delay()));
                timer->async_wait(asio::bind_executor(strand_, [this, timer, event](const boost::system::error_code &ec)
                                                      { handlerCallback_(event); }));
            }
            else
            {
                LOG_DEBUG_FMT("new event: {}", event->name());
                boost::asio::post(strand_, [this, event]()
                                  { handlerCallback_(event); });
            }
        }
    }
}
