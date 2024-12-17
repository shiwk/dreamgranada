#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace mb
{
    const std::string DEFAULT_CONFIG_PATH = "/etc/mb/config";

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

} // namespace mb

#endif