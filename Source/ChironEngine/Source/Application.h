#pragma once

#include "Modules/Module.h"
#include "Enums/ModuleType.h"

class Timer;
class ThreadPool;

class Application
{
public:
    Application() = delete;
    Application(HWND hwnd, HINSTANCE hInstance);
    ~Application();

    bool Init();
    bool Start();
    UpdateStatus Update();
    bool CleanUp();

    // ------------- GETTERS ----------------------

    template<typename M>
    M* GetModule();

    inline ThreadPool* GetMainThreadPool();
    inline const uint64_t GetFrameCount() const;
    inline const float GetDeltaTime() const;
    inline int& GetMaxFrameRate();

private:
    std::vector<std::unique_ptr<Module>> _modules;

    std::unique_ptr<Timer> _timer;
    std::unique_ptr<ThreadPool> _mainThreadPool;

    uint64_t _frameCount;

    int _maxFrameRate;

    float _deltaTime;
};

template<typename M>
inline M* Application::GetModule()
{
    int index = static_cast<int>(ModuleToEnum<M>::value);
    return static_cast<M*>(_modules[index].get());
}

inline ThreadPool* Application::GetMainThreadPool()
{
    return _mainThreadPool.get();
}

inline const uint64_t Application::GetFrameCount() const
{
    return _frameCount;
}

inline const float Application::GetDeltaTime() const
{
    return _deltaTime;
}

inline int& Application::GetMaxFrameRate()
{
    return _maxFrameRate;
}

extern std::unique_ptr<Application> App;
