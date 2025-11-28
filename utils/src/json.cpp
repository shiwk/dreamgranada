#include "json.hpp"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace granada
{
    std::string JsonMember::key()
    {
        return member_.name.GetString();
    }

    JsonValuePtr JsonMember::value()
    {
        return std::make_shared<JsonValue>(doc, member_.value);
    }

    JsonPtr loadJson(const std::string &str)
    {
        return std::make_shared<Json>(str);
    }

    void Json::parse(const std::string &str)
    {
        doc->Parse(str.c_str());
    }

    Json::Json(const std::string &str) : JsonDoc(std::make_shared<rapidjson::Document>())
    {
        parse(str);
    }

    void Json::dump(std::string &out)
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc->Accept(writer);
        out = buffer.GetString();
    }

    bool Json::hasError()
    {
        return doc->HasParseError();
    }

    unsigned int Json::getError()
    {
        return doc->GetParseError();
    }

    template <>
    void JsonValue::addMember(JKey key, const double &d)
    {
        auto alloc = GetAllocator();
        value_.AddMember(rapidjson::Value(key, alloc).Move(),
                         rapidjson::Value(d).Move(),
                         alloc);
    }

    template <>
    void JsonValue::addMember(JKey key, const std::string &s)
    {
        auto alloc = GetAllocator();
        value_.AddMember(rapidjson::Value(key, alloc).Move(),
                         rapidjson::Value(s.c_str(), alloc).Move(),
                         alloc);
    }

    template <>
    void Json::addMember(JKey key, const double &value)
    {
        auto alloc = GetAllocator();
        doc->AddMember(rapidjson::Value(key, alloc).Move(),
                       rapidjson::Value(value).Move(),
                       alloc);
    }

    template <>
    void Json::addMember(JKey key, const std::string &value)
    {
        auto alloc = GetAllocator();
        doc->AddMember(rapidjson::Value(key, alloc).Move(),
                       rapidjson::Value(value.c_str(), alloc).Move(),
                       alloc);
    }

    void Json::addObjMember(JKey key, const JsonValizablePtr obj)
    {
        auto alloc = GetAllocator();
        rapidjson::Value value(rapidjson::kObjectType);
        auto jsonValue = std::make_shared<JsonValue>(doc, value);
        obj->toJson(jsonValue);
        doc->AddMember(rapidjson::Value(key, alloc).Move(),
                       rapidjson::Value(value, alloc).Move(),
                       alloc);
    }

    void Json::addArrayMember(JKey key, const JsonValizablePtr obj)
    {
        auto alloc = GetAllocator();
        rapidjson::Value value(rapidjson::kArrayType);
        auto jsonValue = std::make_shared<JsonValue>(doc, value);
        obj->toJson(jsonValue);
        doc->AddMember(rapidjson::Value(key, alloc).Move(), value, alloc);
    }

    JsonValuePtr Json::get(const std::string &field)
    {
        // if (!doc->HasMember(field.c_str()))
        // {
        //     return nullptr;
        // }

        auto itr = doc->FindMember(field.c_str());
        if (itr == doc->MemberEnd())
        {
            return nullptr;
        }

        auto valuePtr = std::make_shared<JsonValue>(doc, itr->value);
        return valuePtr;
    }

    JsonValuePtr Json::getElement(size_t i)
    {
        if (!doc->IsArray())
        {
            return nullptr;
        }

        auto valuePtr = std::make_shared<JsonValue>(doc, doc->operator[](i));
        return valuePtr;
    }

    JsonValuePtr Json::operator[](const std::string &field)
    {
        return get(field);
    }

    JsonValuePtr Json::operator[](size_t i)
    {
        return getElement(i);
    }

    bool Json::isArray()
    {
        return doc->IsArray();
    }

    bool Json::isObj()
    {
        return doc->IsObject();
    }

    size_t Json::size()
    {
        return doc->Size();
    }

    bool JsonValue::isArray()
    {
        return value_.IsArray();
    }

    bool JsonValue::isObj()
    {
        return value_.IsObject();
    }

    bool JsonValue::has(const std::string &member)
    {
        return value_.HasMember(member.c_str());
    }

    size_t JsonValue::size()
    {
        return value_.Size();
    }

    bool JsonValue::empty()
    {
        return value_.Empty();
    }

    JsonMemberPtr JsonValue::getMember(size_t i)
    {
        if (!value_.IsObject() || i >= value_.MemberCount())
        {
            return nullptr;
        }

        auto memberPtr = std::make_shared<JsonMember>(doc, *(value_.MemberBegin() + i));
        return memberPtr;
    }

    template <>
    int JsonValue::value()
    {
        return value_.Get<int>();
    }

    template <>
    uint64_t JsonValue::value()
    {
        return value_.Get<uint64_t>();
    }

    template <>
    JString JsonValue::value()
    {
        return value_.Get<JString>();
    }

    template <>
    double JsonValue::value()
    {
        return value_.Get<double>();
    }

    template <>
    std::string JsonValue::value()
    {
        return value_.GetString();
    }

    template <>
    bool JsonValue::value()
    {
        return value_.GetBool();
    }

    std::shared_ptr<JsonValue> JsonValue::getElement(size_t i)
    {
        if (!value_.IsArray())
        {
            return nullptr;
        }

        auto valuePtr = std::make_shared<JsonValue>(doc, value_[i]);
        return valuePtr;
    }

    std::shared_ptr<JsonValue> JsonValue::getObj(const std::string &field)
    {
        auto itr = value_.FindMember(field.c_str());
        if (itr == value_.MemberEnd())
        {
            return nullptr;
        }
        auto valuePtr = std::make_shared<JsonValue>(doc, itr->value);
        return valuePtr;
    }

    void JsonValue::addElement(const JsonValizablePtr objPtr)
    {
        auto alloc = GetAllocator();
        rapidjson::Value value(rapidjson::kObjectType);
        auto jsonValue = std::make_shared<JsonValue>(doc, value);
        objPtr->toJson(jsonValue);
        value_.PushBack(value, alloc);
    }

    std::shared_ptr<JsonValue> JsonValue::operator[](const std::string &field)
    {
        return getObj(field);
    }

    std::shared_ptr<JsonValue> JsonValue::operator[](size_t i)
    {
        return getElement(i);
    }

    rapidjson::Document::AllocatorType &JsonDoc::GetAllocator()
    {
        return doc->GetAllocator();
    }

    void JsonDocizable::dump(std::string &out) const
    {
        // auto json = granada::Json(sharedT());
        auto json = std::make_shared<Json>();
        toJson(json);
        json->dump(out);
    }

    void JsonValizable::toJson(JsonValuePtr) const
    {
    }

    void JsonValizable::fromJson(JsonValuePtr)
    {
    }

    void JsonValizable::fromJson(JsonMemberPtr)
    {
    }
    
    void JsonValizable::fromJson(JsonPtr)
    {
    }
}
