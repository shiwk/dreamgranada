#include "ts.hpp"

uint64_t granada::GranadaTimestamp::getCurrentTimestamp()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto duration = now.time_since_epoch();
    return duration_cast<milliseconds>(duration).count();
}