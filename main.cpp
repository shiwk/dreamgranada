#include <iostream>
#include "config.hpp"
#include <glog/logging.h>
#include "worker.hpp"
#include "ioevents.hpp"
#include "events.hpp"

int main(int argc, char* argv[])
{
    std::string configPath = "";
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-c" || std::string(argv[i]) == "--config") {
            configPath = true;
            break;
        }
    }

    granada::ioloop::ioevents ioevents(std::make_shared<granada::Worker>());

    using namespace granada::ioloop;
    using namespace granada;
    std::shared_ptr<granada::ioloop::Event> eventPtr ((Event*)new CommonEvent());
    ioevents.newEvent(eventPtr);
    ioevents.run();
    

    granada::MBConfig mbConfig(configPath);
    DLOG(INFO) << "hello, box";
}