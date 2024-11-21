#pragma once
#include "EditorWindow.h"

#include "DataModels/FileSystem/UID/UID.h"

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
    HierarchyStatus DrawNodeTreeFiltered(GameObject* gameObject);
    void DrawMoveObjectMenu(GameObject* gameObject);
    bool DrawDeleteObjectMenu(GameObject* gameObject);

    bool IsMovable(GameObject* gameObject);
    bool IsDeletable(GameObject* gameObject);

    void Filter(const std::string& filter);
    inline bool IsFiltered(UID uid);

private:
    std::unordered_set<UID> _uidFiltered;
};

inline bool HierarchyWindow::IsFiltered(UID uid)
{
    return _uidFiltered.contains(uid);
}