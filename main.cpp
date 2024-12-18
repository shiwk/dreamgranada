#include <iostream>
#include "config.hpp"
#include <glog/logging.h>

int main(int argc, char* argv[])
{
    std::string configPath = "";
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-c" || std::string(argv[i]) == "--config") {
            configPath = true;
            break;
        }
    }
    mb::MBConfig mbConfig(configPath);
    DLOG(INFO) << "hello, box";
}