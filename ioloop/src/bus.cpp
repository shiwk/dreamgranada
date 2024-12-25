#include "bus.hpp"
#include "handler.hpp"
#include "logger.hpp"

namespace granada
{
    namespace events
    {
        void Bus::onTimeout(asio::steady_timer& timer, std::shared_ptr<asio::io_context> io)
        {

            // 重新设置定时器的过期时间
            timer.expires_at(timer.expiry() + std::chrono::seconds(1));

            // 重新安排定时器的回调
            timer.async_wait([&](const system::error_code& error) {
                LOG_MSG(INFO, "Timer expired");
                if (!error) {
                    onTimeout(timer, io);
                }
            });
        }

        // ioevents::ioevents(EventHandlerPtr event_handler) : io_context_(std::make_unique<boost::asio::io_context>()),
        //                                                                      strand_(boost::asio::make_strand(*io_context_)),
        //                                                                      event_handler_(event_handler) {}
        Bus::Bus() : io_context_(std::make_shared<boost::asio::io_context>()),
                     strand_(boost::asio::make_strand(*io_context_)) {}

        Bus::~Bus()
        {
            LOG_MSG(INFO, "Bus destructor");
            io_context_->stop();
            io_thread_.wait();
        }

        void Bus::setHandlerCallback(std::function<void(EventPtr)> handlerCallback)
        {
            handlerCallback_ = handlerCallback;
        }

        void Bus::start()
        {
            LOG_MSG(INFO, "Bus starting..");
            // auto timer =  std::make_shared<asio::steady_timer>(*io_context_, std::chrono::seconds(1));
            asio::steady_timer timer(*io_context_, std::chrono::seconds(1));
            onTimeout(timer, io_context_);
            io_context_->run();
            // io_thread_ = std::async(std::launch::async, [this]
            //                         { io_context_->run(); });

            // // 启动定时器并设置回调
            // timer->async_wait([&](const system::error_code& error) {
            //     if (!error) {
            //         onTimeout(timer, io_context_);
            //     }
            // });
        }

        // std::shared_ptr<boost::asio::io_context> Bus::getIOContext() const
        // {
        //     return io_context_;
        // }

        asio::io_context& Bus::getIOContext() const
        {
            return *io_context_;
        }

        // void Bus::delayPost(EventPtr event, std::chrono::seconds delay)
        // {
        //     boost::asio::steady_timer timer(*io_context_, delay);
        //     timer.async_wait([this, &event](const boost::system::error_code &ec)
        //                      {
        //                          if (ec)
        //                          {
        //                              LOG_FMT(ERROR, "Error: {}", ec.message());
        //                          }
        //                          else
        //                          {
        //                              LOG_MSG(INFO, "Timer expired");
        //                              handlerCallback_(event);
        //                          }
        //                      });
        // }

        void Bus::postEvent(EventPtr event)
        {
            boost::asio::post(strand_, [this, event]()
                              { 
                                handlerCallback_(event);
                               });
        }
    }
}
