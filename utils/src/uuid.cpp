#include "uuid.hpp"
#include "ts.hpp"
#include <sstream>
#include <iomanip>

using namespace granada;


uuid GranadaUID::gen(const std::string &prefix, const uint64_t ts)
{
    uint64_t timestamp = ts;
    if (timestamp == 0)
    {
        timestamp = GranadaTimestamp::getCurrentTimestamp();
    }
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

    return std::move(ss.str());
}

GranadaUID &GranadaUID::instance()
{
    static GranadaUID uid;
    return uid;
}