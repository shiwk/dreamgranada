#include "json.hpp"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace granada
{
    JsonPtr loadJson(const std::string &str)
    {
        return std::make_shared<Json>(str);
    }

    void Json::parse(const std::string &str)
    {
        doc_->Parse(str.c_str());
    }

    Json::Json(const std::string &str) : doc_(std::make_shared<rapidjson::Document>())
    {
        parse(str);
    }

    void Json::dump(std::string &out)
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc_->Accept(writer);
        out = buffer.GetString();
    }

    bool Json::hasError()
    {
        return doc_->HasParseError();
    }

    unsigned int Json::getError()
    {
        return doc_->GetParseError();
    }

    JsonValuePtr Json::get(const std::string &field)
    {
        if (!doc_->HasMember(field.c_str()))
        {
            return nullptr;
        }

        auto valuePtr = std::make_shared<JsonValue>(doc_->operator[](field.c_str()));
        return valuePtr;
    }

    JsonValuePtr Json::get(size_t i)
    {
        if (!doc_->IsArray())
        {
            return nullptr;
        }

        auto valuePtr = std::make_shared<JsonValue>(doc_->operator[](i));
        return valuePtr;
    }

    JsonValuePtr Json::operator[](const std::string &field)
    {
        return get(field);
    }

    JsonValuePtr Json::operator[](size_t i)
    {
        return get(i);
    }

    bool Json::isArray()
    {
        return doc_->IsArray();
    }

    bool Json::isObj()
    {
        return doc_->IsObject();
    }

    size_t Json::size()
    {
        return doc_->Size();
    }

    bool JsonValue::isArray()
    {
        return value_.IsArray();
    }

    bool JsonValue::isObj()
    {
        return value_.IsObject();
    }

    bool JsonValue::has(const std::string& member)
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

    std::shared_ptr<JsonValue> JsonValue::get(size_t i)
    {
        if (!value_.IsArray())
        {
            return nullptr;
        }

        auto valuePtr = std::make_shared<JsonValue>(value_[i]);
        return valuePtr;
    }

    std::shared_ptr<JsonValue> JsonValue::get(const std::string &field)
    {
        if (!value_.HasMember(field.c_str()))
        {
            return nullptr;
        }

        auto valuePtr = std::make_shared<JsonValue>(value_[field.c_str()]);
        return valuePtr;
    }

    std::shared_ptr<JsonValue> JsonValue::operator[](const std::string &field)
    {
        return get(field);
    }

    std::shared_ptr<JsonValue> JsonValue::operator[](size_t i)
    {
        return get(i);
    }
}
