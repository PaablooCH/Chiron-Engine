#pragma once
#include "EditorWindow.h"

class GameObject;
class ComponentWindow;

struct AddComponentAction;

class InspectorWindow : public EditorWindow
{
public:
    InspectorWindow();
    ~InspectorWindow() override;

private:
    void DrawWindowContent(const std::shared_ptr<CommandList>& commandList) override;
    void DrawGameObjectInfo();
    void DrawComponentsWindows(const std::shared_ptr<CommandList>& commandList);
    void DrawAddComponent();

    void FillComponentsWindows();

    void AddMeshRendererComponent();

private:
    std::vector<std::unique_ptr<ComponentWindow>> _componentsWindows;
    std::vector<AddComponentAction> _actions;

    GameObject* _lastSelected;
};

