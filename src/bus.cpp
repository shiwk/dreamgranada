#include "bus.hpp"
#include "logger.hpp"

namespace granada
{
    namespace events
    {
        void Bus::onTimeout(std::shared_ptr<asio::steady_timer> &timer)
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
                                  onTimeout(timer); });
        }

        Bus::~Bus()
        {
            LOG_INFO("Bus destructor");
            // auto& busEngine = getBusEngine();
            // busEngine->stop();
            // io_thread_.wait();
        }
        
        void Bus::stop()
        {
            auto& busEngine = getBusEngine();
            busEngine->stop();
        }

        // void Bus::setBusStop(std::function<void(EventPtr)> handlerCallback)
        // {
        //     handlerCallback_ = handlerCallback;
        // }

        void Bus::newBusStop(BusStopPtr stop)
        {
            busStops_.emplace_back(stop);
        }

        void Bus::start()
        {
            LOG_INFO("Bus starting..");
            auto& busEngine = getBusEngine();
            std::shared_ptr<asio::steady_timer> timer = std::make_shared<asio::steady_timer>(*busEngine, std::chrono::seconds(1));
            onTimeout(timer);
            busEngine->run();
        }

        Bus::io_contextPtr& Bus::getBusEngine()
        {
            static io_contextPtr engine = std::make_shared<asio::io_context>();
            return engine;
        }

        strand<io_context::executor_type> Bus::getStrand()
        {
            auto& busEngine = getBusEngine();
            static strand<io_context::executor_type> strand(asio::make_strand(*busEngine));
            return strand;
        }

        void Bus::board(EventPtr event)
        {
            if (event->delay() > 0)
            {
                LOG_DEBUG_FMT("delayed event: {} {}ms ", event->name(), event->delay());
                for (auto &stop : busStops_)
                {
                    std::shared_ptr<asio::steady_timer> timer = std::make_shared<asio::steady_timer>(*getBusEngine(), std::chrono::milliseconds(event->delay()));
                    timer->async_wait(asio::bind_executor(getStrand(), [timer, stop, event](const boost::system::error_code &ec)
                                                        { stop->onStop(event); }));
                }
                
            }
            else
            {
                LOG_DEBUG_FMT("new event: {}", event->name());
                for (auto &stop : busStops_)
                {
                    asio::post(getStrand(), [stop, event]() { stop->onStop(event); });
                }
            }
        }
    }
}
