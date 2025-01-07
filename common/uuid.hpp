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
        static uint64_t getCurrentTimestamp()
        {
            using namespace std::chrono;
            auto now = system_clock::now();
            auto duration = now.time_since_epoch();
            return duration_cast<milliseconds>(duration).count();
        }

    public:
        std::string gen(const std::string &prefix)
        {
            uint64_t timestamp = getCurrentTimestamp();
            std::lock_guard<std::mutex> lock(mutex_);
            if (timestamp != lastTs_)
            {
                lastTs_ = timestamp;
                counter_ = 0;
            }

            uint64_t currentCounter = counter_++;

            std::stringstream ss;
            if (!prefix.empty())
            {
                ss << prefix << "-";
            }

            ss << std::setw(8) << std::setfill('0') << std::hex << timestamp
               << std::setw(4) << std::setfill('0') << std::hex << (currentCounter & 0xFFFF);

            return ss.str();
        }

        static GranadaUID &instance()
        {
            static GranadaUID uuid;
            return uuid;
        }
    };
}
#endif
