#include "Pch.h"
#include "InspectorWindow.h"

#include "Application.h"

#include "Modules/ModuleScene.h"

#include "Factories/ComponentWindowFactory.h"

#include "DataModels/Components/TransformComponent.h"

#include "DataModels/Window/EditorWindow/SubWindows/Inspector/TransformComponentWindow.h"

#include "DataModels/GameObject/GameObject.h"

InspectorWindow::InspectorWindow() : EditorWindow(ICON_FA_CIRCLE_INFO " Inspector", ImGuiWindowFlags_AlwaysAutoResize), _lastSelected(nullptr)
{
}

InspectorWindow::~InspectorWindow()
{
}

void InspectorWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
    auto selected = App->GetModule<ModuleScene>()->GetSelectedGameObject();
    if (selected)
    {
        if (selected != _lastSelected)
        {
            _lastSelected = selected;
            _componentsWindows.clear();
            FillComponentsWindows();

        }
        else if (_lastSelected->HowManyComponentsHas() != _componentsWindows.size())
        {
            _componentsWindows.clear();
            FillComponentsWindows();
        }
        DrawGameObjectInfo();
        DrawComponentsWindows(commandList);
        DrawAddComponent();
    }
}

void InspectorWindow::DrawGameObjectInfo()
{
    bool isRoot = _lastSelected->GetParent() == nullptr;
    if (!isRoot)
    {
        bool& enabled = _lastSelected->IsEnabled();
        ImGui::Checkbox("###enableGameObject", &enabled);
    
        ImGui::SameLine();
    }

    float inputTextWidth = ImGui::GetContentRegionAvail().x;
    if (!isRoot)
    {
        inputTextWidth -= ImGui::CalcTextSize("Static").x + 35.0f;
    }
    ImGui::PushItemWidth(inputTextWidth > 50.0f ? inputTextWidth : 50.0f);
    std::string name = _lastSelected->GetName();
    if (ImGui::InputText("##GameObject", name.data(), 32, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
    {
        _lastSelected->SetName(name);
    }
    ImGui::PopItemWidth();

    if (!isRoot)
    {
        ImGui::SameLine();
        bool isStatic = _lastSelected->IsStatic();
        if (ImGui::Checkbox("###staticGameObject", &isStatic))
        {
            _lastSelected->SetStatic(isStatic);
        }
        ImGui::SameLine();
        ImGui::Text("Static");
    }
}

void InspectorWindow::DrawComponentsWindows(const std::shared_ptr<CommandList>& commandList)
{
    if (ImGui::BeginChild("Child", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY))
    {
        ImGui::SeparatorText("Components");
        for (auto& componentWindow : _componentsWindows)
        {
            componentWindow->Draw(commandList);
        }
        ImGui::EndChild();
    }
}

void InspectorWindow::DrawAddComponent()
{
    ImGui::Separator();
    ImGui::Text("Add Compoent implementarlo");
}

void InspectorWindow::FillComponentsWindows()
{
    for (auto component : _lastSelected->GetComponents())
    {
        _componentsWindows.push_back(ComponentWindowFactory::CreateComponentWindow(component));
    }
}
