#include "Pch.h"
#include "InspectorWindow.h"

InspectorWindow::InspectorWindow() : EditorWindow("Inspector", ImGuiWindowFlags_AlwaysAutoResize)
{
}

InspectorWindow::~InspectorWindow()
{
}

void InspectorWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
}
