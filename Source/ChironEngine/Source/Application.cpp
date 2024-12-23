#include "Pch.h"
#include "Application.h"

#include "Modules/ModuleCamera.h"
#include "Modules/ModuleFileSystem.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleID3D12.h"
#include "Modules/ModuleInput.h"
#include "Modules/ModuleProgram.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleScene.h"
#include "Modules/ModuleWindow.h"

#include "DataModels/Timer/Timer.h"

#if OPTICK
    #include "Optick/optick.h"
#endif // OPTICK

Application::Application(HWND hwnd, HINSTANCE hInstance) : _frameCount(0), _deltaTime(0), _maxFrameRate(200)
{
    _modules.resize(static_cast<int>(ModuleType::LAST));
    _modules[static_cast<int>(ModuleToEnum<ModuleWindow>::value)] = std::make_unique<ModuleWindow>(hwnd, hInstance);
    _modules[static_cast<int>(ModuleToEnum<ModuleID3D12>::value)] = std::make_unique<ModuleID3D12>();
    _modules[static_cast<int>(ModuleToEnum<ModuleFileSystem>::value)] = std::make_unique<ModuleFileSystem>();
    _modules[static_cast<int>(ModuleToEnum<ModuleProgram>::value)] = std::make_unique<ModuleProgram>();
    _modules[static_cast<int>(ModuleToEnum<ModuleInput>::value)] = std::make_unique<ModuleInput>(hwnd);
    _modules[static_cast<int>(ModuleToEnum<ModuleResources>::value)] = std::make_unique<ModuleResources>();
    _modules[static_cast<int>(ModuleToEnum<ModuleCamera>::value)] = std::make_unique<ModuleCamera>();
    _modules[static_cast<int>(ModuleToEnum<ModuleScene>::value)] = std::make_unique<ModuleScene>();
    _modules[static_cast<int>(ModuleToEnum<ModuleRender>::value)] = std::make_unique<ModuleRender>();
    _modules[static_cast<int>(ModuleToEnum<ModuleEditor>::value)] = std::make_unique<ModuleEditor>();

    _timer = std::make_unique<Timer>();
}

Application::~Application()
{
    _modules.clear();
}

bool Application::Init()
{
    if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
    {
        LOG_ERROR("Co Initialize Failed");
        return false;
    }

    for (const std::unique_ptr<Module>& module : _modules)
    {
        if (!module->Init())
        {
            return false;
        }
    }

    return true;
}

bool Application::Start()
{
    _timer->Start();

    for (const std::unique_ptr<Module>& module : _modules)
    {
        if (!module->Start())
        {
            return false;
        }
    }

    return true;
}

UpdateStatus Application::Update()
{
    auto beginFrame = _timer->Read();

    _frameCount++;

    for (const std::unique_ptr<Module>& module : _modules)
    {
        UpdateStatus result = module->PreUpdate();
        if (result != UpdateStatus::UPDATE_CONTINUE)
        {
            return result;
        }
    }

    for (const std::unique_ptr<Module>& module : _modules)
    {
        UpdateStatus result = module->Update();
        if (result != UpdateStatus::UPDATE_CONTINUE)
        {
            return result;
        }
    }

    for (const std::unique_ptr<Module>& module : _modules)
    {
        UpdateStatus result = module->PostUpdate();
        if (result != UpdateStatus::UPDATE_CONTINUE)
        {
            return result;
        }
    }

    auto endFrame = _timer->Read();

    auto ms = endFrame - beginFrame;

    double maxMs = 1000.0 / _maxFrameRate;
    if (ms < maxMs)
    {
#if OPTICK
        OPTICK_CATEGORY("Sleep", Optick::Category::None);
#endif // DEBUG
        std::chrono::duration<double, std::milli> sleepTime(maxMs - ms);
        auto sleepTimeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(sleepTime);
        if (sleepTimeDuration.count() > 0.f)
        {
            timeBeginPeriod(1);
            std::this_thread::sleep_for(sleepTimeDuration);
            timeEndPeriod(1);
        }
        endFrame = _timer->Read();
        ms = endFrame - beginFrame;
    }
    _deltaTime = static_cast<float>(ms / 1000);

    return UpdateStatus::UPDATE_CONTINUE;
}

bool Application::CleanUp()
{
    GetModule<ModuleID3D12>()->Flush();

    std::ranges::reverse_view reverseModules = std::ranges::reverse_view{ _modules };
    for (const std::unique_ptr<Module>& module : reverseModules)
    {
        if (!module->CleanUp())
        {
            return false;
        }
    }

    return true;
}