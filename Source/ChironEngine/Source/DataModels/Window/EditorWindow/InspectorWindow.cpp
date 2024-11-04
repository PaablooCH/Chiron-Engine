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
    if (ImGui::InputText("##NameGameObject", &name, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
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

        if (ImGui::BeginTable("TagTable", 2))
        {

            ImGui::TableSetupColumn("###tagFirstCol", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("###tagSecondCol", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            ImGui::Dummy(ImVec2(0, 1));
            ImGui::Text("Tag");

            ImGui::TableNextColumn();
            std::string tag = _lastSelected->GetTag();
            if (ImGui::InputText("##TagGameObject", &tag, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
            {
                _lastSelected->SetTag(tag);
            }

            ImGui::EndTable();
        }
    }
}

void InspectorWindow::DrawComponentsWindows(const std::shared_ptr<CommandList>& commandList)
{
    if (!_componentsWindows.empty())
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
        auto componentWindow = ComponentWindowFactory::CreateComponentWindow(component);
        if (componentWindow)
        {
            _componentsWindows.push_back(std::move(componentWindow));
        }
    }
}
