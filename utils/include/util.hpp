#ifndef UTIL_HPP
#define UTIL_HPP

#include <sstream>
#include <iomanip>
#include <limits>
#include <bitset>

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
                std::string binaryString = std::bitset<32>(t).to_string();
                return std::move("0B" + binaryString);
            }
        };
    } // namespace utils
} // namespace granada

#endif