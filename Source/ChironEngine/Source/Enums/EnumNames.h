#pragma once

#include <string>
#include <vector>

inline std::vector<std::string> CreateEnumNamesVector(const std::string& names) 
{
    std::vector<std::string> namesVector;
    size_t start = 0, end;
    while ((end = names.find(',', start)) != std::string::npos) 
    {
        namesVector.push_back(names.substr(start, end - start));
        start = end + 1;
    }
    namesVector.push_back(names.substr(start));
    return namesVector;
}

#define DECLARE_ENUM_WITH_TYPE(E, T, ...)                                                                           \
    enum class E : T                                                                                                \
    {                                                                                                               \
        __VA_ARGS__,                                                                                                \
        UNKNOWN                                                                                                     \
    };                                                                                                              \
    std::vector<std::string> E##VectorName = CreateEnumNamesVector(#__VA_ARGS__);                                   \
    size_t operator*(E enumTmp) { (void) enumTmp; return E##VectorName.size(); }                                    \
    std::string operator~(E enumTmp) { return E##VectorName[static_cast<int>(enumTmp)]; }                           \
    std::string operator+(std::string &&str, E enumTmp) { return str + E##VectorName[static_cast<int>(enumTmp)]; }  \
    std::string operator+(E enumTmp, std::string &&str) { return E##VectorName[static_cast<int>(enumTmp)] + str; }  \
    std::string &operator+=(std::string &str, E enumTmp)                                                            \
    {                                                                                                               \
        str += E##VectorName[static_cast<int>(enumTmp)];                                                            \
        return str;                                                                                                 \
    }                                                                                                               \
    inline std::string ToString(E enumTmp)                                                                          \
    {                                                                                                               \
        int index = static_cast<int>(enumTmp);                                                                      \
        if (index >= 0 && index < E##VectorName.size())                                                             \
        {                                                                                                           \
            return E##VectorName[index];                                                                            \
        }                                                                                                           \
        return "UNKNOWN";                                                                                           \
    }

#define DECLARE_ENUM_NAMES(E, ...) DECLARE_ENUM_WITH_TYPE(E, int32_t, __VA_ARGS__)