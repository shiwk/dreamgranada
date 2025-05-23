#include "json.hpp"

namespace granada
{
    JsonPtr Json::load(const std::string &str)
    {
        JsonPtr j = std::shared_ptr<Json>(new Json);
        j->parse(str);
        return j;
    }

    void Json::parse(const std::string &str)
    {
        doc_->Parse(str.c_str());
    }

    bool Json::hasError()
    {
        return doc_->HasParseError();
    }

    unsigned int Json::getError()
    {
        return doc_->GetParseError();
    }

    JsonValuePtr Json::get(JString field)
    {
        if (!doc_->HasMember(field))
        {
            return nullptr;
        }

        auto valuePtr = std::make_shared<JsonValue>(doc_->operator[](field));
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

    bool Json::isArray()
    {
        return doc_->IsArray();
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
    int JsonValue::get()
    {
        return value_.Get<int>();
    }

    template <>
    uint64_t JsonValue::get()
    {
        return value_.Get<uint64_t>();
    }

    template <>
    JString JsonValue::get()
    {
        return value_.Get<JString>();
    }

    template <>
    double JsonValue::get()
    {
        return value_.Get<double>();
    }

    template <>
    std::string JsonValue::get()
    {
        return value_.GetString();
    }

    template <>
    bool JsonValue::get()
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

    std::shared_ptr<JsonValue> JsonValue::get(JString field)
    {
        if (!value_.HasMember(field))
        {
            return nullptr;
        }

        auto valuePtr = std::make_shared<JsonValue>(value_[field]);
        return valuePtr;
    }
}