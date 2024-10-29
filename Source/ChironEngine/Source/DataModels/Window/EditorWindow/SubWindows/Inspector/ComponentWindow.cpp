#include "Pch.h"
#include "ComponentWindow.h"

#include "DataModels/Components/Component.h"
#include "DataModels/GameObject/GameObject.h"

#include <sstream>

void ComponentWindow::Draw(const std::shared_ptr<CommandList>& commandList)
{
    if (CollapsingHeader())
    {
        DrawWindowContent(commandList);
    }
}

ComponentWindow::ComponentWindow(std::string name, Component* component, bool disableEnable, bool disableRemove) : 
    SubWindow(name), _component(component), _windowUID(0U), _disableEnable(disableEnable), _disableRemove(disableRemove)
{
    _flags |= ImGuiTreeNodeFlags_AllowOverlap;
}

bool ComponentWindow::CollapsingHeader()
{
    bool open = ImGui::CollapsingHeader(_name.c_str(), _flags);
    if (!_disableEnable)
    {
        DrawEnable();
    }
    if (!_disableRemove)
    {
        DrawRemoveComponent();
    }
    return open;
}

void ComponentWindow::DrawEnable()
{
    if (_component)
    {
        std::stringstream ss;
        ss << "##Enabled" << _windowUID;

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 75);
        bool& enable = _component->IsEnabled();
        ImGui::Checkbox(ss.str().c_str(), &enable);
        if (ImGui::BeginItemTooltip()) 
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("Enable/Disable");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}

void ComponentWindow::DrawRemoveComponent()
{
    if (_component)
    {
        std::stringstream ss;
        ss << ICON_FA_TRASH "###" << _windowUID;

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 45);
        if (ImGui::Button(ss.str().c_str(), ImVec2(30, 22)))
        {
            _component->GetOwner()->RemoveComponent(_component);
            _component = nullptr;
        }
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("Remove Component");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}