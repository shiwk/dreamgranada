#ifndef UUID_HPP
#define UUID_HPP

#include <atomic>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <string>
#include <mutex>
#include "common.hpp"

namespace granada
{
    using uuid = std::string;
    class GranadaUID
    {
    private:
        int counter_;
        ll lastTs_;
        std::mutex mutex_;
        static uint64_t getCurrentTimestamp();

    public:
        std::string gen(const std::string &prefix);
        static GranadaUID &instance();
    };
}    
#endif
