#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "json.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>

namespace granada
{
    /**
     * Generic configuration value that wraps a JSON value.
     * Provides type-safe access to configuration values.
     */
    class ConfigValue
    {
    public:
        ConfigValue() = default;
        explicit ConfigValue(JsonValuePtr value) : value_(value) {}

        // Type-safe accessors
        std::string asString(const std::string &defaultValue = "") const;
        int asInt(int defaultValue = 0) const;
        double asDouble(double defaultValue = 0.0) const;
        bool asBool(bool defaultValue = false) const;
        std::vector<ConfigValue> asArray() const;

        // Check if value exists
        bool isValid() const { return value_ != nullptr; }

        // Nested access
        ConfigValue operator[](const std::string &key) const;
        ConfigValue operator[](size_t index) const;

    private:
        JsonValuePtr value_;
    };

    /**
     * Configuration provider interface.
     * Different sources (JSON file, env vars, etc.) implement this.
     */
    class ConfigProvider
    {
    public:
        virtual ~ConfigProvider() = default;
        virtual bool load() = 0;
        virtual ConfigValue get(const std::string &path) const = 0;
        virtual bool has(const std::string &path) const = 0;
    };
    MAKE_SHARED_PTR(ConfigProvider)

    /**
     * JSON file configuration provider.
     * Loads configuration from a JSON file.
     */
    class JsonConfigProvider : public ConfigProvider
    {
    public:
        explicit JsonConfigProvider(const std::string &filePath);
        explicit JsonConfigProvider(JsonPtr json) : json_(json) {}
        ~JsonConfigProvider() override = default;

        bool load() override;
        ConfigValue get(const std::string &path) const override;
        bool has(const std::string &path) const override;

    private:
        std::string filePath_;
        JsonPtr json_;

        JsonValuePtr resolvePath(const std::string &path) const;
    };

    /**
     * Environment variable configuration provider.
     * Loads configuration from environment variables with a given prefix.
     */
    class EnvConfigProvider : public ConfigProvider
    {
    public:
        explicit EnvConfigProvider(const std::string &prefix = "GRANADA_")
            : prefix_(prefix) {}
        ~EnvConfigProvider() override = default;

        bool load() override;
        ConfigValue get(const std::string &path) const override;
        bool has(const std::string &path) const override;

    private:
        std::string prefix_;
        std::unordered_map<std::string, std::string> envMap_;

        std::string envKey(const std::string &path) const;
    };

    /**
     * Configuration manager.
     * Manages multiple providers and provides merged configuration.
     * Later providers override earlier ones.
     */
    class Config
    {
    public:
        static Config &instance();

        // Add a configuration provider
        void addProvider(ConfigProviderPtr provider);

        // Load all providers
        bool loadAll();

        // Get configuration value by path (e.g., "database.host")
        ConfigValue get(const std::string &path) const;

        // Check if a configuration value exists
        bool has(const std::string &path) const;

        // Convenience: get with default value
        std::string getString(const std::string &path, const std::string &defaultValue = "") const;
        int getInt(const std::string &path, int defaultValue = 0) const;
        double getDouble(const std::string &path, double defaultValue = 0.0) const;
        bool getBool(const std::string &path, bool defaultValue = false) const;
        std::vector<ConfigValue> getArray(const std::string &path) const;

        // Clear all providers
        void clear();

    private:
        Config() = default;
        std::vector<ConfigProviderPtr> providers_;
    };
}

#endif