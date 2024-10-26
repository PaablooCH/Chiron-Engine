#pragma once
#include "DataModels/Window/Window.h"

#include <ImGui/imgui.h>

class EditorWindow : public Window
{
public:
    virtual ~EditorWindow();

    void Draw(const std::shared_ptr<CommandList>& commandList) override;

protected:
    EditorWindow(const std::string& name, ImGuiWindowFlags flags = ImGuiWindowFlags_None);

    virtual void DrawWindowContent(const std::shared_ptr<CommandList>& commandList) = 0;

    ImGuiWindowFlags _flags;

private:
    bool _focused;
};
