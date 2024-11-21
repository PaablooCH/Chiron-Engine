#include "Pch.h"
#include "Utils.h"

void Chiron::Utils::ThrowIfFailed(HRESULT hr, const std::string& message) noexcept(false)
{
    if (FAILED(hr))
    {
        LOG_ERROR("Error: " + GetErrorMessage(hr) + " (HRESULT: 0x" + std::to_string(hr) + ")");
        throw std::runtime_error("");
    }
}

std::string Chiron::Utils::WStringToString(const std::wstring& wstr)
{
    // Get the buffer size
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);

    // Reserve space
    std::string str(bufferSize, 0);

    // Parse
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], bufferSize, nullptr, nullptr);

    return str;
}

std::wstring Chiron::Utils::StringToWString(const std::string& str)
{
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(bufferSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], bufferSize);
    return wstr;
}

HANDLE Chiron::Utils::CreateEventHandle()
{
    HANDLE event;

    event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(event && "Failed to create event.");

    return event;
}

std::string Chiron::Utils::GetErrorMessage(HRESULT hr)
{
    LPWSTR errorText = nullptr;

    // Get the message
    DWORD formatResult = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&errorText, 0, nullptr);

    std::string errorString;
    if (formatResult > 0)
    {
        // Parse to string
        std::wstring wErrorText(errorText);
        errorString = WStringToString(wErrorText);

        // Free memory
        LocalFree(errorText);
    }
    else
    {
        errorString = "Unknown error";
    }

    return errorString;
}