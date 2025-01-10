#ifndef UUID_HPP
#define UUID_HPP


#include <string>
#include <mutex>
#include "common.hpp"

namespace granada
{
    using uuid = std::string;
    class GranadaUID
    {
    private:
        int counter_;
        ll lastTs_;
        std::mutex mutex_;

    public:
        
        uuid gen(const std::string &prefix, const uint64_t ts=0);
        static GranadaUID &instance();
    };
}    
#endif
