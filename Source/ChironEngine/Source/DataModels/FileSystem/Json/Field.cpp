#include "Pch.h"
#include "Field.h"

Field::Field(rapidjson::Value& value, rapidjson::Document& documentJson) : _value(value), _documentJson(documentJson)
{
}

Field Field::operator[](unsigned it)
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
