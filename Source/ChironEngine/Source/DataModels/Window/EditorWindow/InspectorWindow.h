#pragma once
#include "EditorWindow.h"

class InspectorWindow : public EditorWindow
{
public:
    InspectorWindow();
    ~InspectorWindow() override;

private:
    void DrawWindowContent(const std::shared_ptr<CommandList>& commandList) override;

private:
};

