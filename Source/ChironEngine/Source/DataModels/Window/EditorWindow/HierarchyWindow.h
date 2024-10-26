#pragma once
#include "EditorWindow.h"

class GameObject;

class HierarchyWindow : public EditorWindow
{
public:
    HierarchyWindow();
    ~HierarchyWindow() override;

private:
    enum class HierarchyStatus
    {
        SUCCESS,
        HAS_CHANGES
    };
    void DrawWindowContent(const std::shared_ptr<CommandList>& commandList) override;
    void DrawSearchBar();
    HierarchyStatus DrawNodeTree(GameObject* gameObject);
    void DrawMoveObjectMenu(GameObject* gameObject);
    bool DrawDeleteObjectMenu(GameObject* gameObject);

    bool IsMovable(GameObject* gameObject);
    bool IsDeletable(GameObject* gameObject);

private:
};
