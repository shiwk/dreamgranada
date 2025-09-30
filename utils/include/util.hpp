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
        class Format
        {
        public:
            template <class T>
            static inline std::string dumpHex(T t)
            {
                std::stringstream ss;
                ss << "0X" << std::setw(4) << std::setfill('0') << std::hex << t;
                return std::move(ss.str());
            }
            
            template <class T>
            static inline std::string dumpB(T t)
            {
                std::string binaryString = std::bitset<64>(t).to_string();
                return std::move("0B" + binaryString);
            }
        };

        class NumUtil
        {
            public:
            template<class num>
            static size_t minBytesRequired(num value)
            {
                if (value == 0) return 1;

                int bits = sizeof(num) * 8 - __builtin_clz(value);
                return (bits + 7) / 8;
            }
        };

        class DateUtil
        {
            public:
            static time_t dateToTimestamp(const std::string &dateStr, const std::string &dateFmt);
            static uint64_t msUntilNext(int target_hour, int target_minute = 0, int target_second = 0);
        };

        class RandomUtil
        {
        public:
            static int getRandomInt(int min, int max);
            static double getRandomDouble(double min, double max);
        };
    } // namespace utils
} // namespace granada

#endif