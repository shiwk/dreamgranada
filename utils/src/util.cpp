#include "util.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>

time_t granada::utils::date::dateToTimestamp(const std::string &dateStr, const std::string &dateFmt)
{
    std::tm tm = {};
    std::istringstream iss(dateStr);
    iss >> std::get_time(&tm, dateFmt.c_str());
    if (iss.fail())
    {
        return -1;
    }
    time_t timestamp = timegm(&tm);
    return timestamp;
}

const std::string granada::utils::date::timestampToDate(const std::time_t &tt, const std::string &dateFmt)
{
    std::tm tm_buf;
    ::gmtime_r(&tt, &tm_buf);
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, dateFmt.c_str());
    return oss.str();
}

std::time_t granada::utils::date::getNowTs()
{
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

std::tm granada::utils::date::getGmtNow()
{
    std::time_t now_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm gm_tm;
    ::gmtime_r(&now_c, &gm_tm);
    return gm_tm;
}

std::tm granada::utils::date::getGmtTime(std::time_t tt)
{
    std::tm gm_tm;
    ::gmtime_r(&tt, &gm_tm);
    return gm_tm;
}

uint64_t granada::utils::date::msUntilNext(int target_hour, int target_minute, int target_second)
{
    time_t now_c = granada::utils::date::getNowTs();

    std::tm local_tm = *std::localtime(&now_c);

    std::tm target_tm = local_tm;
    target_tm.tm_hour = target_hour;
    target_tm.tm_min = target_minute;
    target_tm.tm_sec = target_second;

    time_t target_time_t = timegm(&target_tm);

    if (target_time_t <= now_c)
    {
        target_tm.tm_mday += 1;
        target_time_t = timegm(&target_tm);
    }

    auto diff_milliseconds = static_cast<uint64_t>(target_time_t - now_c) * 1000;

    return diff_milliseconds;
}

uint64_t granada::utils::date::lastWeekDayNotLaterThan(uint64_t ts, int target_weekday, int target_hour, int target_minute, int target_second)
{
    std::time_t tt = static_cast<std::time_t>(ts);
    std::tm tm_buf;
    ::gmtime_r(&tt, &tm_buf);

    int current_weekday = tm_buf.tm_wday;
    int days_difference = (current_weekday - target_weekday + 7) % 7;

    if (days_difference == 0)
    {
        if (tm_buf.tm_hour < target_hour ||
            (tm_buf.tm_hour == target_hour && tm_buf.tm_min < target_minute) ||
            (tm_buf.tm_hour == target_hour && tm_buf.tm_min == target_minute && tm_buf.tm_sec < target_second))
        {
            days_difference = 7;
        }
    }

    tm_buf.tm_mday -= days_difference;
    tm_buf.tm_hour = target_hour;
    tm_buf.tm_min = target_minute;
    tm_buf.tm_sec = target_second;

    std::time_t target_time_t = timegm(&tm_buf);
    return static_cast<uint64_t>(target_time_t);
}

uint64_t granada::utils::date::lastMonthDayNotLaterThan(uint64_t ts, int target_monthday, int target_hour, int target_minute, int target_second)
{
    std::time_t tt = static_cast<std::time_t>(ts);

    std::tm tm_buf;
    ::gmtime_r(&tt, &tm_buf);

    if (tm_buf.tm_mday > target_monthday ||
        (tm_buf.tm_mday == target_monthday &&
         (tm_buf.tm_hour > target_hour ||
          (tm_buf.tm_hour == target_hour && tm_buf.tm_min > target_minute) ||
          (tm_buf.tm_hour == target_hour && tm_buf.tm_min == target_minute && tm_buf.tm_sec >= target_second))))
    {
        tm_buf.tm_hour = target_hour;
        tm_buf.tm_min = target_minute;
        tm_buf.tm_sec = target_second;
        tm_buf.tm_mday = target_monthday;
        std::time_t target_time_t = timegm(&tm_buf);
        return static_cast<uint64_t>(target_time_t);
    }

    tm_buf.tm_hour = target_hour;
    tm_buf.tm_min = target_minute;
    tm_buf.tm_sec = target_second;
    int mon = tm_buf.tm_mon;

    while (true)
    {
        tm_buf.tm_mon = --mon;
        tm_buf.tm_mday = target_monthday;
        std::time_t target_time_t = timegm(&tm_buf);
        if (tm_buf.tm_mon == mon && tm_buf.tm_mday == target_monthday)
        {
            return static_cast<uint64_t>(target_time_t);
        }
    }
}

int granada::utils::nums::getRandomInt(int min, int max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

double granada::utils::nums::getRandomDouble(double min, double max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

std::string granada::utils::string::toLower(const std::string &s)
{
    std::string result;
    result.resize(s.length());
    std::transform(s.begin(), s.end(), result.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return result;
}

std::string granada::utils::string::trim(const std::string &s)
{
    const char *whitespace = " \n\r\t\f\v";
    size_t start = s.find_first_not_of(whitespace);
    size_t end = s.find_last_not_of(whitespace);
    return (start == std::string::npos || end == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

bool granada::utils::string::startsWith(const std::string &s, const std::string &prefix)
{
    return prefix.size() <= s.size() && std::equal(prefix.begin(), prefix.end(), s.begin());
}
