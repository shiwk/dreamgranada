#include "util.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>

time_t granada::utils::DateUtil::dateToTimestamp(const std::string &dateStr, const std::string &dateFmt)
{
    std::tm tm = {};
    std::istringstream iss(dateStr);
    iss >> std::get_time(&tm, dateFmt.c_str());
    if (iss.fail()) {
        return -1;
    }
    time_t timestamp = std::mktime(&tm);
    return timestamp;
}


uint64_t granada::utils::DateUtil::msUntilNext(int target_hour, int target_minute, int target_second)
{
    auto now = std::chrono::system_clock::now();
    time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::tm local_tm = *std::localtime(&now_c);

    std::tm target_tm = local_tm;
    target_tm.tm_hour = target_hour;
    target_tm.tm_min = target_minute;
    target_tm.tm_sec = target_second;

    time_t target_time_t = std::mktime(&target_tm);

    if (target_time_t <= now_c) {
        target_tm.tm_mday += 1;
        target_time_t = std::mktime(&target_tm);
    }

    auto diff_milliseconds = static_cast<uint64_t>(target_time_t - now_c) * 1000;

    return diff_milliseconds;
}

int granada::utils::RandomUtil::getRandomInt(int min, int max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

double granada::utils::RandomUtil::getRandomDouble(double min, double max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}
