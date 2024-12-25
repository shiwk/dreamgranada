#include <iostream>
#include "config.hpp"
#include <glog/logging.h>
#include "ioloop.hpp"
#include "bus.hpp"
#include "postman.hpp"
#include "logger.hpp"
#include "fetcher.hpp"


using namespace granada;
int main(int argc, char *argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::SetStderrLogging(google::INFO);

    // std::string configPath = "";
    // for (int i = 1; i < argc; ++i) {
    //     if (std::string(argv[i]) == "-c" || std::string(argv[i]) == "--config") {
    //         configPath = true;
    //         break;
    //     }
    // }
    // granada::MBConfig mbConfig(configPath);
    // LOG(FATAL) << "This is a FATAL log.";

    auto eventBus = std::make_shared<events::Bus>();

    using namespace granada::events;
    using namespace granada;
    std::future<void> f = std::async(std::launch::async, [&]
                                     { eventBus->start(); });
    auto ioloop = std::make_shared<IOLoop>(eventBus);
    auto fetcher = std::make_shared<Fetcher>(eventBus);
    ioloop->subscribe(fetcher);
    granada::events::EventPtr eventPtr ((Event*)new CommonEvent());
    eventBus->postEvent(eventPtr);

    f.wait();

    LOG_MSG(INFO, "hello, granada");
}