#pragma once

#include "Field.h"
#include "rapidjson/stringbuffer.h"

class Json
{
public:
    Json(rapidjson::Document& document);
    ~Json();

    bool ToJson(const char* buffer);
    rapidjson::StringBuffer ToBuffer();

    Field operator[](const char* key) const;

private:
    rapidjson::Document& _document;
};
