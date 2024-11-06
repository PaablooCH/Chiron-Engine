#include "Pch.h"
#include "SceneWindow.h"

#include "Application.h"

#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"

#include "DataModels/Camera/Camera.h"
#include "DataModels/DX12/CommandList/CommandList.h"

#include "DataModels/DX12/DescriptorAllocator/DescriptorAllocator.h"
#include "DataModels/DX12/Resource/Texture.h"

SceneWindow::SceneWindow() : EditorWindow(ICON_FA_BOX_OPEN " Scene", ImGuiWindowFlags_AlwaysAutoResize |
    ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse), _currentWidth(0),
    _currentHeight(0)
{
}

SceneWindow::~SceneWindow()
{
}

void SceneWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
    ManageResize();
    auto sceneTexture = App->GetModule<ModuleRender>()->GetSceneTexture();
    commandList->TransitionBarrier(sceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    ImGui::Image((ImTextureID)(sceneTexture->GetShaderResourceView().GetGPUDescriptorHandle().ptr),
        ImGui::GetContentRegionAvail());
}

void SceneWindow::ManageResize()
{
    auto availableRegion = ImGui::GetContentRegionAvail();
    bool sizeChanged = _currentWidth != availableRegion.x || _currentHeight != availableRegion.y;

    if (sizeChanged) // window was resized
    {
        ModuleCamera* camera = App->GetModule<ModuleCamera>();
        camera->GetCamera()->SetAspectRatio(availableRegion.x / availableRegion.y);
        _currentWidth = availableRegion.x;
        _currentHeight = availableRegion.y;
    }
}