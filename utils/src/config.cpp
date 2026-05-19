#include "config.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace granada
{
    // ===== ConfigValue =====

    std::string ConfigValue::asString(const std::string &defaultValue) const
    {
        if (!value_)
            return defaultValue;
        try
        {
            return value_->value<std::string>();
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    int ConfigValue::asInt(int defaultValue) const
    {
        if (!value_)
            return defaultValue;
        try
        {
            return static_cast<int>(value_->value<double>());
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    double ConfigValue::asDouble(double defaultValue) const
    {
        if (!value_)
            return defaultValue;
        try
        {
            return value_->value<double>();
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    bool ConfigValue::asBool(bool defaultValue) const
    {
        if (!value_)
            return defaultValue;
        try
        {
            return value_->value<bool>();
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    std::vector<ConfigValue> ConfigValue::asArray() const
    {
        std::vector<ConfigValue> result;
        if (!value_)
            return result;
        try
        {
            for (size_t i = 0; i < value_->size(); ++i)
            {
                result.emplace_back((*value_)[i]);
            }
        }
        catch (...)
        {
            // Not an array
        }
        return result;
    }

    ConfigValue ConfigValue::operator[](const std::string &key) const
    {
        if (!value_)
            return ConfigValue();
        try
        {
            return ConfigValue((*value_)[key]);
        }
        catch (...)
        {
            return ConfigValue();
        }
    }

    ConfigValue ConfigValue::operator[](size_t index) const
    {
        if (!value_)
            return ConfigValue();
        try
        {
            return ConfigValue((*value_)[index]);
        }
        catch (...)
        {
            return ConfigValue();
        }
    }

    // ===== JsonConfigProvider =====

    JsonConfigProvider::JsonConfigProvider(const std::string &filePath)
        : filePath_(filePath), json_(std::make_shared<Json>())
    {
    }

    bool JsonConfigProvider::load()
    {
        std::ifstream file(filePath_);
        if (!file.is_open())
        {
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        json_ = loadJson(buffer.str());
        return json_ != nullptr;
    }

    JsonValuePtr JsonConfigProvider::resolvePath(const std::string &path) const
    {
        if (!json_)
            return nullptr;

        std::vector<std::string> keys;
        std::string current;
        for (char c : path)
        {
            if (c == '.')
            {
                if (!current.empty())
                {
                    keys.push_back(current);
                    current.clear();
                }
            }
            else
            {
                current += c;
            }
        }
        if (!current.empty())
        {
            keys.push_back(current);
        }

        if (keys.empty())
            return nullptr;

        JsonValuePtr currentValue = (*json_)[keys[0]];
        for (size_t i = 1; i < keys.size() && currentValue != nullptr; ++i)
        {
            currentValue = (*currentValue)[keys[i]];
        }

        return currentValue;
    }

    ConfigValue JsonConfigProvider::get(const std::string &path) const
    {
        return ConfigValue(resolvePath(path));
    }

    bool JsonConfigProvider::has(const std::string &path) const
    {
        return resolvePath(path) != nullptr;
    }

    // ===== EnvConfigProvider =====

    std::string EnvConfigProvider::envKey(const std::string &path) const
    {
        std::string key = prefix_;
        for (char c : path)
        {
            if (c == '.')
            {
                key += '_';
            }
            else
            {
                key += static_cast<char>(std::toupper(c));
            }
        }
        return key;
    }

    bool EnvConfigProvider::load()
    {
        envMap_.clear();
        // We don't pre-load all env vars; instead we look them up on demand
        return true;
    }

    ConfigValue EnvConfigProvider::get(const std::string &path) const
    {
        std::string key = envKey(path);
        const char *value = std::getenv(key.c_str());
        if (value == nullptr)
        {
            // Try without the prefix as fallback
            std::string plainKey;
            for (char c : path)
            {
                if (c == '.')
                {
                    plainKey += '_';
                }
                else
                {
                    plainKey += static_cast<char>(std::toupper(c));
                }
            }
            value = std::getenv(plainKey.c_str());
            if (value == nullptr)
            {
                return ConfigValue();
            }
        }

        // Try to parse as JSON first (for complex values)
        try
        {
            auto json = loadJson(value);
            if (json != nullptr && !json->hasError())
            {
                return ConfigValue(json->get("value"));
            }
        }
        catch (...)
        {
            // Not JSON, treat as string
        }

        // Try to parse as number
        try
        {
            size_t pos = 0;
            double num = std::stod(value, &pos);
            if (pos == strlen(value))
            {
                auto json = std::make_shared<Json>();
                json->addMember("value", num);
                return ConfigValue(json->get("value"));
            }
        }
        catch (...)
        {
            // Not a number
        }

        // Try to parse as bool
        std::string lowerValue(value);
        std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);
        if (lowerValue == "true" || lowerValue == "false")
        {
            auto json = std::make_shared<Json>();
            json->addMember("value", lowerValue == "true" ? 1.0 : 0.0);
            return ConfigValue(json->get("value"));
        }

        // Treat as string
        auto json = std::make_shared<Json>();
        json->addMember("value", std::string(value));
        return ConfigValue(json->get("value"));
    }

    bool EnvConfigProvider::has(const std::string &path) const
    {
        std::string key = envKey(path);
        return std::getenv(key.c_str()) != nullptr;
    }

    // ===== Config (Manager) =====

    Config &Config::instance()
    {
        static Config config;
        return config;
    }

    void Config::addProvider(ConfigProviderPtr provider)
    {
        providers_.push_back(provider);
    }

    bool Config::loadAll()
    {
        bool allOk = true;
        for (auto &provider : providers_)
        {
            if (!provider->load())
            {
                allOk = false;
            }
        }
        return allOk;
    }

    ConfigValue Config::get(const std::string &path) const
    {
        // Traverse in reverse order: later providers override earlier ones
        for (auto it = providers_.rbegin(); it != providers_.rend(); ++it)
        {
            if ((*it)->has(path))
            {
                return (*it)->get(path);
            }
        }
        return ConfigValue();
    }

    bool Config::has(const std::string &path) const
    {
        for (const auto &provider : providers_)
        {
            if (provider->has(path))
            {
                return true;
            }
        }
        return false;
    }

    std::string Config::getString(const std::string &path, const std::string &defaultValue) const
    {
        return get(path).asString(defaultValue);
    }

    int Config::getInt(const std::string &path, int defaultValue) const
    {
        return get(path).asInt(defaultValue);
    }

    double Config::getDouble(const std::string &path, double defaultValue) const
    {
        return get(path).asDouble(defaultValue);
    }

    bool Config::getBool(const std::string &path, bool defaultValue) const
    {
        return get(path).asBool(defaultValue);
    }

    std::vector<ConfigValue> Config::getArray(const std::string &path) const
    {
        return get(path).asArray();
    }

    void Config::clear()
    {
        providers_.clear();
    }
}
