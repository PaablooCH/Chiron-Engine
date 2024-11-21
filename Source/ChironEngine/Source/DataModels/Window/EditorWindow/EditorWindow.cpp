#include "Pch.h"
#include "EditorWindow.h"

EditorWindow::EditorWindow(const std::string& name, ImGuiWindowFlags flags) : Window(name), _flags(flags), _focused(false)
{
}

EditorWindow::~EditorWindow()
{
}

void EditorWindow::Draw(const std::shared_ptr<CommandList>& commandList)
{
    if (_enabled)
    {
        if (ImGui::Begin(_name.c_str(), &_enabled, _flags))
        {
            DrawWindowContent(commandList);
            _focused = ImGui::IsWindowFocused() && ImGui::IsWindowHovered();
        }
        ImGui::End();
    }
    // The call to ImGui::Begin can change the value of io_enabled
    // so using "else" will cause the window to remain focused for an extra frame
    if (!_enabled)
    {
        _focused = false;
    }
}