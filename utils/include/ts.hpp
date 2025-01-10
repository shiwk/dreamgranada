#ifndef TS_HPP
#define TS_HPP

#include <chrono>

namespace granada
{
    class GranadaTimestamp
    {
        public:
            static uint64_t getCurrentTimestamp(); 
    };
    
} // namespace granada



#endif