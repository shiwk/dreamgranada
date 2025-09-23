#include "util.hpp"
#include <sstream>
#include <iomanip>

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