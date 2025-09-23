#ifndef UTIL_HPP
#define UTIL_HPP

#include <sstream>
#include <iomanip>
#include <limits>
#include <bitset>
#include <ctime>

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

        class DateUtil
        {
            public:
            static time_t dateToTimestamp(const std::string &dateStr, const std::string &dateFmt);
        };
    } // namespace utils
} // namespace granada

#endif