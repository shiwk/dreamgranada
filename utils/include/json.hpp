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
        
        std::shared_ptr<JsonValue> get(const std::string&);
        std::shared_ptr<JsonValue> get(size_t);
        template <class T>
        T value();

        std::shared_ptr<JsonValue> operator[](const std::string&);
        std::shared_ptr<JsonValue> operator[](size_t);

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
        template <class T>
        Json(std::shared_ptr<T> t): doc_(std::make_shared<rapidjson::Document>())
        {
            t->toJsonMember(doc_);
        }

        Json(const std::string &);
        bool hasError();
        unsigned int getError();
        Json() : doc_(std::make_shared<rapidjson::Document>()) {}
        ~Json()
        {
            LOG_DEBUG("Json destroyed");
        }

        JsonValuePtr get(const std::string &);
        JsonValuePtr get(size_t i);
        JsonValuePtr operator[](const std::string &field);
        JsonValuePtr operator[](size_t i);


        bool isArray();

        bool isObj();

        size_t size();

        void dump(std::string &out);

    private:
        void parse(const std::string &str);
        std::shared_ptr<rapidjson::Document> doc_;
        template <class T>
        bool get(rapidjson::Document &, T &t);
    };

    MAKE_SHARED_PTR(Json)

    template<class T>
    class JsonDocizable : public std::enable_shared_from_this<T>
    {
    public:
        ~JsonDocizable() = default;
        virtual void toJsonMember(std::shared_ptr<rapidjson::Document>) const = 0;
        void dump(std::string &out) const
        {
            auto json = granada::Json(sharedT());
            json.dump(out);
        }
    private:
        std::shared_ptr<T const> sharedT() const
        {
            return this->shared_from_this();
        }
    };
    class JsonValizable
    {
    public:
        ~JsonValizable() = default;
        virtual void toJsonMember(rapidjson::Value&, rapidjson::Document::AllocatorType &)  const = 0;
    };

    extern JsonPtr loadJson(const std::string &);    
}

#endif