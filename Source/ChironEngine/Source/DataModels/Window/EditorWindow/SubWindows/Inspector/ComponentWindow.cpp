#include "Pch.h"
#include "ComponentWindow.h"

#include "DataModels/Components/Component.h"

#include "DataModels/GameObject/GameObject.h"

#include "DataModels/FileSystem/UID/UIDGenerator.h"

#include <sstream>

void ComponentWindow::Draw(const std::shared_ptr<CommandList>& commandList)
{
    if (CollapsingHeader())
    {
        DrawWindowContent(commandList);
    }
}

ComponentWindow::ComponentWindow(std::string name, Component* component, bool disableEnable, bool disableRemove) :
    SubWindow(name), _component(component), _windowUID(Chiron::UIDGenerator::GenerateUID()), _disableEnable(disableEnable), 
    _disableRemove(disableRemove)
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
        std::ostringstream oss;

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 55);
        bool enable = _component->IsEnabled();
        oss << (enable ? ICON_FA_EYE : ICON_FA_EYE_SLASH) << "##Enabled" << _windowUID;
        if (ImGui::Button(oss.str().c_str()))
        {
            _component->SetEnabled(!enable);
        }
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
        std::ostringstream oss;
        oss << ICON_FA_TRASH "###" << _windowUID;

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25);
        if (ImGui::Button(oss.str().c_str()))
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
        ImGui::Dummy(ImVec2(30, 0));
    }
}