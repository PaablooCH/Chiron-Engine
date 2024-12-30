#include "Pch.h"
#include "CameraSubWindow.h"

#include "Application.h"

#include "Modules/ModuleCamera.h"

#include "DataModels/Camera/Camera.h"

#include "DataModels/FileSystem/Json/Json.h"

CameraSubWindow::CameraSubWindow() : SubWindow("Camera")
{
}

CameraSubWindow::~CameraSubWindow()
{
}

void CameraSubWindow::Save(Json& json)
{
    auto camera = json["Camera"];
    auto fov = App->GetModule<ModuleCamera>()->GetCamera()->GetFOV();
    camera["FOV"] = fov;
}

void CameraSubWindow::Load(const Json& json)
{
    auto camera = json["Camera"];
    App->GetModule<ModuleCamera>()->GetCamera()->SetFOV(camera["FOV"]);
}

void CameraSubWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
    auto availableX = ImGui::GetContentRegionAvail().x;

    auto camera = App->GetModule<ModuleCamera>()->GetCamera();
    auto fov = camera->GetFOV();
    const char* label = "FOV:";
    ImVec2 labelSize = ImGui::CalcTextSize(label);
    float sliderWidth = availableX - labelSize.x - ImGui::GetStyle().ItemInnerSpacing.x - 5;
    ImGui::TextUnformatted(label);
    ImGui::SameLine();
    ImGui::PushItemWidth(sliderWidth);
    if (ImGui::SliderFloat("##fov", &fov, 0.001f, 3.124f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
    {
        camera->SetFOV(fov);
    }
    ImGui::PopItemWidth();
}