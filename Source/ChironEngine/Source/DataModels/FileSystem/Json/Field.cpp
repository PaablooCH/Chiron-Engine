#include "Pch.h"
#include "Field.h"

Field::Field(rapidjson::Value& value, rapidjson::Document& documentJson) : _value(value), _documentJson(documentJson)
{
}

Field Field::operator[](unsigned it) const
{
    if (!_value.IsArray())
    {
        _value.SetArray();
    }
    size_t size = _value.GetArray().Size();
    if (size <= it)
    {
        for (size_t i = size; i <= it; i++)
        {
            _value.PushBack(rapidjson::Value(), _documentJson.GetAllocator());
        }
    }
    return Field(_value[it], _documentJson);
}

Field Field::operator[](const char* key) const
{
    if (!_value.IsObject())
    {
        _value.SetObject();
    }

    if (!_value.HasMember(key))
    {
        _value.AddMember(rapidjson::StringRef(key), rapidjson::Value(), _documentJson.GetAllocator());
    }
    return Field(_value[key], _documentJson);
}