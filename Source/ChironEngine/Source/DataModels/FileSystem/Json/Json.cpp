#include "Pch.h"
#include "Json.h"

#include "rapidjson/prettywriter.h"

Json::Json(rapidjson::Document& document) : _document(document)
{
    _document.SetObject();
}

Json::~Json()
{
}

bool Json::ToJson(const char* buffer)
{
    _document.Parse<rapidjson::kParseNanAndInfFlag>(buffer);
    if (_document.HasParseError())
    {
        LOG_ERROR("Error parsing JSON: {}", rapidjson::GetParseErrorFunc(_document.GetParseError()));
        return false;
    }
    return true;
}

rapidjson::StringBuffer Json::ToBuffer()
{
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
    _document.Accept(writer);
    return sb;
}

Field Json::operator[](const char* key)
{
    if (!_document.HasMember(key))
    {
        _document.AddMember(rapidjson::StringRef(key), rapidjson::Value(), _document.GetAllocator());
    }
    
    return Field(_document[key], _document);
}
