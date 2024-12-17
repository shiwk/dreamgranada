#include <iostream>
#include "config.hpp"

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
    std::cout << "hello, box" << std::endl;
}