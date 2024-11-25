#pragma once

#include "rapidjson/document.h"
#include "DataModels/FileSystem/UID/UID.h"

class Field
{
public:
    Field(rapidjson::Value& value, rapidjson::Document& documentJson);

    Field operator[](unsigned it);
    Field operator[](const char* key);

    inline operator bool() const;
    inline operator int() const;
    inline operator unsigned() const;
    inline operator float() const;
    inline operator long long() const;
    inline operator UID() const;
    inline operator double() const;
    inline operator std::string() const;

    inline void operator=(bool b);
    inline void operator=(int i);
    inline void operator=(unsigned u);
    inline void operator=(float f);
    inline void operator=(long long ll);
    inline void operator=(UID uid);
    inline void operator=(double d);
    inline void operator=(const char* c);

private:
    rapidjson::Value& _value;
    rapidjson::Document& _documentJson;
};

inline Field::operator bool() const
{
    return _value.IsBool() ? _value.GetBool() : false;
}

inline Field::operator int() const
{
    return _value.IsInt() ? _value.GetInt() : 0;
}

inline Field::operator unsigned() const
{
    return _value.IsUint() ? _value.GetUint() : 0;
}

inline Field::operator float() const
{
    return _value.IsFloat() ? _value.GetFloat() : 0.f;
}

inline Field::operator long long() const
{
    return _value.IsInt64() ? _value.GetInt64() : 0LL;
}

inline Field::operator UID() const
{
    return _value.IsUint64() ? _value.GetUint64() : 0ULL;
}

inline Field::operator double() const
{
    return _value.IsDouble() ? _value.GetDouble() : 0.0;
}

inline Field::operator std::string() const
{
    return _value.IsString() ? _value.GetString() : "";
}

inline void Field::operator=(bool b)
{
    _value.SetBool(b);
}

inline void Field::operator=(int i)
{
    _value.SetInt(i);
}

inline void Field::operator=(unsigned u)
{
    _value.SetUint(u);
}

inline void Field::operator=(float f)
{
    _value.SetFloat(f);
}

inline void Field::operator=(long long ll)
{
    _value.SetInt64(ll);
}

inline void Field::operator=(UID uid)
{
    _value.SetUint64(uid);
}

inline void Field::operator=(double d)
{
    _value.SetDouble(d);
}

inline void Field::operator=(const char* c)
{
    _value.SetString(c, _documentJson.GetAllocator());
}
