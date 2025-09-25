#ifndef JSON_HPP
#define JSON_HPP

#include <rapidjson/document.h>
#include "common.hpp"
#include "logger.hpp"

namespace granada
{
    using JString = const char *;
    using JKey = const char *;
    class JsonValue;
    class JsonValizable;
    
    class JsonDoc
    {
        public:
        JsonDoc(std::shared_ptr<rapidjson::Document> doc) : doc(doc){}
        protected:
            rapidjson::Document::AllocatorType& GetAllocator();
            std::shared_ptr<rapidjson::Document> doc;
    };

    class JsonMember : JsonDoc
    {
    public:
        JsonMember(std::shared_ptr<rapidjson::Document> doc, rapidjson::Value::Member &member) : JsonDoc(doc), member_(member) {}
        ~JsonMember() = default;
        
        std::string key();
        std::shared_ptr<JsonValue> value();

    private:
        rapidjson::Value::Member &member_;
    };

    MAKE_SHARED_PTR(JsonMember)


    class JsonValue : JsonDoc
    {
    public:
        JsonValue(std::shared_ptr<rapidjson::Document> doc, rapidjson::Value &value) : JsonDoc(doc), value_(value) {}
        ~JsonValue() = default;
        
        std::shared_ptr<JsonValue> getObj(const std::string&);
        std::shared_ptr<JsonValue> getElement(size_t);
        std::shared_ptr<JsonMember> getMember(size_t);

        template <class T>
        T value();

        template<class T>
        void addMember(JKey key, const T& value);

        template<class T>
        std::shared_ptr<std::vector<std::shared_ptr<T>>> getMembers(size_t max = 0)
        {
            auto memberList = std::make_shared<std::vector<std::shared_ptr<T>>>();
            for (auto& member : value_.GetObject()) {
                if (max && memberList->size() >= max)
                {
                    break;
                }
                auto jsonMember = std::make_shared<JsonMember>(doc, member);
                auto t = std::make_shared<T>();
                t->fromJson(jsonMember);
                memberList->push_back(t);
            }

            return memberList;
        }

        void addElement(const std::shared_ptr<JsonValizable>);

        std::shared_ptr<JsonValue> operator[](const std::string&);
        std::shared_ptr<JsonValue> operator[](size_t);

        bool isArray();
        bool isObj();
        bool has(const std::string &);
        size_t size();
        bool empty();

    private:
        rapidjson::Value &value_;
    };
    MAKE_SHARED_PTR(JsonValue)

    class JsonValizable
    {
    public:
        ~JsonValizable() = default;
        virtual void toJson(JsonValuePtr) const = 0;
        virtual void fromJson(JsonValuePtr) = 0;
        virtual void fromJson(JsonMemberPtr) = 0;
    };
    MAKE_SHARED_PTR(JsonValizable)


    class Json : JsonDoc
    {
    public:
        Json() : JsonDoc(std::make_shared<rapidjson::Document>()) {
            doc->SetObject();
        }

        Json(const std::string &);
        bool hasError();
        unsigned int getError();
        ~Json()
        {
            LOG_DEBUG("Json destroyed");
        }

        JsonValuePtr get(const std::string &);
        JsonValuePtr getElement(size_t);
        JsonValuePtr operator[](const std::string &field);
        JsonValuePtr operator[](size_t);

        bool isArray();
        bool isObj();
        size_t size();
        void dump(std::string &out);

        template<class T>
        void addMember(JKey key, const T& value);
        void addObjMember(JKey key, const JsonValizablePtr);
        void addArrayMember(JKey key, const JsonValizablePtr);

    private:
        void parse(const std::string &str);
    };

    MAKE_SHARED_PTR(Json)

    class JsonDocizable
    {
    public:
        ~JsonDocizable() = default;
        virtual void toJson(JsonPtr) const = 0;
        void dump(std::string &out) const;
    };

    extern JsonPtr loadJson(const std::string &);    
}

#endif