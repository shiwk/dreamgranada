#ifndef UTIL_HPP
#define UTIL_HPP

#include <sstream>
#include <iomanip>
#include <limits>
#include <bitset>
#include <ctime>
#include <cstdint>

namespace granada
{
    namespace utils
    {
        namespace format
        {
            template <class T>
            inline std::string dumpHex(T t)
            {
                std::stringstream ss;
                ss << "0X" << std::setw(4) << std::setfill('0') << std::hex << t;
                return ss.str();
            }
            
            template <class T>
            inline std::string dumpB(T t)
            {
                std::string binaryString = std::bitset<64>(t).to_string();
                return "0B" + binaryString;
            }
        }

        namespace string
        {
            extern std::string toLower(const std::string&);
            extern std::string trim(const std::string&);
            extern bool startsWith(const std::string&, const std::string& prefix);
        };

        namespace nums
        {
            template<class num>
            inline size_t minBytesRequired(num value)
            {
                if (value == 0) return 1;

                int bits = sizeof(num) * 8 - __builtin_clz(value);
                return (bits + 7) / 8;
            }
            extern int getRandomInt(int min, int max);
            extern double getRandomDouble(double min, double max);
        };

        namespace date
        {
            extern time_t dateToTimestamp(const std::string &dateStr, const std::string &dateFmt);
            extern const std::string timestampToDate(const std::time_t &, const std::string &dateFmt);
            extern time_t getNowTs();
            extern std::tm getGmtNow();
            extern std::tm getGmtTime(std::time_t);
            extern uint64_t msUntilNext(int target_hour, int target_minute = 0, int target_second = 0);
            extern uint64_t lastWeekDayNotLaterThan(uint64_t ts, int target_weekday, int target_hour=0, int target_minute = 0, int target_second = 0);
            extern uint64_t lastMonthDayNotLaterThan(uint64_t ts, int target_monthday, int target_hour=0, int target_minute = 0, int target_second = 0);
        };
    } // namespace utils
} // namespace granada

#endif