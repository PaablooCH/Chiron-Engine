#pragma once

#include "debug_draw.hpp"

namespace Chiron
{
    class Utils
    {
    public:

        Utils() = delete;

        // ------------- DIRECTX ----------------------

        // Helper utility converts D3D API failures into exceptions.
        static void ThrowIfFailed(HRESULT hr, const std::string& message = "") noexcept(false);

        static inline void OpenLink(const std::string& url);

        // ------------- PARSE ----------------------

        static std::string WStringToString(const std::wstring& wstr);
        static std::wstring StringToWString(const std::string& str);

        // ------------- CONTAINERS ----------------------

        template<typename T>
        static inline void TryFrontPop(std::queue<T>& queue, T& element);

        template<typename T>
        static inline void TryFrontPop(std::queue<T>& queue, T& element, bool& success);

        // ------------- MEMORY ----------------------

        template <typename T>
        static __forceinline T AlignUpWithMask(T value, size_t mask)
        {
            return (T)(((size_t)value + mask) & ~mask);
        }

        template <typename T>
        static __forceinline T AlignDownWithMask(T value, size_t mask)
        {
            return (T)((size_t)value & ~mask);
        }

        template <typename T>
        static __forceinline T AlignUp(T value, size_t alignment)
        {
            return AlignUpWithMask(value, alignment - 1);
        }

        template <typename T>
        static __forceinline T AlignDown(T value, size_t alignment)
        {
            return AlignDownWithMask(value, alignment - 1);
        }

        template <typename T>
        static __forceinline bool IsAligned(T value, size_t alignment)
        {
            return 0 == ((size_t)value & (alignment - 1));
        }

        template <typename T>
        static __forceinline T DivideByMultiple(T value, size_t alignment)
        {
            return (T)((value + alignment - 1) / alignment);
        }

        template <typename T>
        static __forceinline bool IsPowerOfTwo(T value)
        {
            return 0 == (value & (value - 1));
        }

        template <typename T>
        static __forceinline bool IsDivisible(T value, T divisor)
        {
            return (value / divisor) * divisor == value;
        }

        // ------------- HANDLES ----------------------

        static HANDLE CreateEventHandle();

        // ------------- DEBUG DRAW ----------------------

        static inline const ddVec3& ddConvert(const DirectX::SimpleMath::Vector3& v);
        static inline const ddMat4x4& ddConvert(const DirectX::SimpleMath::Matrix& m);

    private:
        static std::string GetErrorMessage(HRESULT hr);
    };

    inline void Chiron::Utils::OpenLink(const std::string& url)
    {
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    template<typename T>
    inline void Utils::TryFrontPop(std::queue<T>& queue, T& element)
    {
        if (!queue.empty())
        {
            element = std::move(queue.front());
            queue.pop();
        }
    }

    template<typename T>
    inline void Utils::TryFrontPop(std::queue<T>& queue, T& element, bool& success)
    {
        if (!queue.empty())
        {
            element = std::move(queue.front());
            queue.pop();
            success = true;
        }
        else
        {
            success = false;
        }
    }

    inline const ddVec3& Utils::ddConvert(const DirectX::SimpleMath::Vector3& v)
    {
        return reinterpret_cast<const ddVec3&>(v);
    }

    inline const ddMat4x4& Utils::ddConvert(const DirectX::SimpleMath::Matrix& m)
    {
        return reinterpret_cast<const ddMat4x4&>(m);
    }
}