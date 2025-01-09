#include "uuid.hpp"

using namespace granada;

uint64_t GranadaUID::getCurrentTimestamp()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto duration = now.time_since_epoch();
    return duration_cast<milliseconds>(duration).count();
}

std::string GranadaUID::gen(const std::string &prefix)
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

GranadaUID &GranadaUID::instance()
{
    static GranadaUID uid;
    return uid;
}