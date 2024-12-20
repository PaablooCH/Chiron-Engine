#include "Pch.h"
#include "ModuleCamera.h"

#include "DataModels/Camera/Camera.h"

#ifdef PROFILE
    #include "Optick/optick.h"
#endif // OPTICK

ModuleCamera::ModuleCamera()
{
}

ModuleCamera::~ModuleCamera()
{
}

bool ModuleCamera::Init()
{
    _camera = std::make_unique<Camera>(Vector3(0.f, 2.f, 5.f), Vector3::Forward, Vector3::Up, DirectX::XM_PIDIV4, 0.1f, 10000.f);
    return true;
}

UpdateStatus ModuleCamera::PreUpdate()
{
    return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleCamera::Update()
{
#ifdef PROFILE
    OPTICK_CATEGORY("UpdateCamera", Optick::Category::Camera);
#endif // DEBUG
    _camera->Update();

    return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleCamera::PostUpdate()
{
    return UpdateStatus::UPDATE_CONTINUE;
}

bool ModuleCamera::CleanUp()
{
    return true;
}