#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace granada
{
    const std::string DEFAULT_CONFIG_PATH = "/etc/granada/config";

    class MBConfig
    {
    private:
        /* data */
        std::string path_;
    public:
        MBConfig() : path_(DEFAULT_CONFIG_PATH)
        {
        }

        MBConfig(const std::string configPath) : path_(configPath)
        {
        }

        ~MBConfig()
        {
        }
    };

} // namespace granada

#endif