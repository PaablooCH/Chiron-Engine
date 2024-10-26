#include "Pch.h"
#include "Formatter.h"

#include "DataModels/GameObject/GameObject.h"

namespace Chiron::detail
{
    namespace
    {
        const char* replaceKey = "{}";
    }

    size_t FindPositionKey(const std::string& format)
    {
        return format.find(replaceKey);
    }

    bool Format(std::string& format, GameObject* arg)
    {
        return Format(format, arg->GetName());
    }

    bool Format(std::string& format, int arg)
    {
        return Format(format, std::to_string(arg));
    }

    bool Format(std::string& format, unsigned int arg)
    {
        return Format(format, std::to_string(arg));
    }

    bool Format(std::string& format, float arg)
    {
        return Format(format, std::to_string(arg));
    }

    bool Format(std::string& format, const char* arg)
    {
        return Format(format, std::string(arg));
    }

    bool Format(std::string& format, const std::string& arg)
    {
        size_t keyPosition = FindPositionKey(format);
        if (keyPosition != std::string::npos)
        {
            format.replace(keyPosition, strlen(replaceKey), arg);
            return true;
        }
        return false;
    }

    bool Format(std::string& format, bool arg)
    {
        return Format(format, std::to_string(arg));
    }

    bool Format(std::string& format, unsigned long long arg)
    {
        return Format(format, std::to_string(arg));
    }
}