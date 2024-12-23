#include "Pch.h"
#include "ModuleInput.h"

#ifdef PROFILE
    #include "Optick/optick.h"
#endif // OPTICK

ModuleInput::ModuleInput(HWND hwnd) : _mousePos(Vector2::Zero), _captureMousePos(false)
{
    _keyboard = std::make_unique<DirectX::Keyboard>();
    _mouse = std::make_unique<DirectX::Mouse>();
    _mouse->SetWindow(hwnd);
}

ModuleInput::~ModuleInput()
{
}

bool ModuleInput::Init()
{
    const auto& mouseState = _mouse->GetState();
    _mousePos = Vector2(static_cast<float>(mouseState.x), static_cast<float>(mouseState.y));

    return true;
}

UpdateStatus ModuleInput::PreUpdate()
{
#ifdef PROFILE
    OPTICK_CATEGORY("PreUpdateInput", Optick::Category::Input);
#endif // DEBUG
    const auto& mouseState = _mouse->GetState();
    if (mouseState.positionMode == DirectX::Mouse::MODE_ABSOLUTE)
    {
        if (_captureMousePos)
        {
            _mousePos = Vector2(static_cast<float>(mouseState.x), static_cast<float>(mouseState.y));
            _captureMousePos = false;
        }
    }
    return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleInput::Update()
{
    return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleInput::PostUpdate()
{
#ifdef PROFILE
    OPTICK_CATEGORY("PostUpdateInput", Optick::Category::Input);
#endif // DEBUG
    const auto& mouseState = _mouse->GetState();
    _mousePos = Vector2(static_cast<float>(mouseState.x), static_cast<float>(mouseState.y));

    return UpdateStatus::UPDATE_CONTINUE;
}

bool ModuleInput::CleanUp()
{
    return true;
}