#ifndef IOLOOP_HPP
#define IOLOOP_HPP

#include <boost/asio.hpp>
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
            // std::future<void> io_thread_;
            std::function<void(EventPtr)> handlerCallback_;
            std::vector<BusStopPtr> busStops_;
            static void onTimeout(std::shared_ptr<asio::steady_timer> &);
            static strand<io_context::executor_type> getStrand();

        public:
            ~Bus();
            // void setBusStop(std::function<void(EventPtr)>);
            void newBusStop(BusStopPtr );
            void stop();
            void board(EventPtr);
            void start();
            static io_contextPtr &getBusEngine();
        };

        MAKE_SHARED_PTR(Bus);
        MAKE_WEAK_PTR(Bus);
    }

}

#endif