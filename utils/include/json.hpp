#ifndef JSON_HPP
#define JSON_HPP

#include <rapidjson/document.h>
#include "common.hpp"
#include "logger.hpp"

namespace granada
{
    using JString = const char *;
    class JsonValue
    {
    public:
        JsonValue(const rapidjson::Value &value) : value_(value) {}
        ~JsonValue()
        {
            // LOG_DEBUG("JsonValue destroyed");
        }
        std::shared_ptr<JsonValue> get(const char *field);

        template <class T>
        T get();

        std::shared_ptr<JsonValue> get(size_t i);

        bool isArray();
        bool isObj();
        bool has(const std::string &);

        size_t size();

        bool empty();

    private:
        const rapidjson::Value &value_;
    };
    MAKE_SHARED_PTR(JsonValue)

    class Json
    {
    public:
        Json(const std::string &);
        bool hasError();
        unsigned int getError();
        Json() : doc_(std::make_shared<rapidjson::Document>()) {}
        ~Json()
        {
            LOG_DEBUG("Json destroyed");
        }

        JsonValuePtr get(const char *field);

        JsonValuePtr get(size_t i);

        bool isArray();

        size_t size();

        void toString(std::string &out);

    private:
        void parse(const std::string &str);
        std::shared_ptr<rapidjson::Document> doc_;
        template <class T>
        bool get(rapidjson::Document &, T &t);
    };

    MAKE_SHARED_PTR(Json)

    extern JsonPtr loadJson(const std::string &);

    template <class T>
    void toJson(T t, JsonPtr);
}

#endif