#include "Pch.h"
#include "HierarchyWindow.h"

#include "Application.h"

#include "Modules/ModuleScene.h"

#include "DataModels/GameObject/GameObject.h"

#include <sstream>

HierarchyWindow::HierarchyWindow() : EditorWindow(ICON_FA_SITEMAP " Hierarchy", ImGuiWindowFlags_AlwaysAutoResize)
{
}

HierarchyWindow::~HierarchyWindow()
{
}

void HierarchyWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
    DrawSearchBar();

    ImGui::Separator();
    CHIRON_TODO("If too slow create a vector with all the gameobject and update it when anything in it change???");
    auto root = App->GetModule<ModuleScene>()->GetRoot();
    if (root)
    {
        if (_uidFiltered.empty())
        {
            DrawNodeTree(root);
        }
        else
        {
            DrawNodeTreeFiltered(root);
        }
        CHIRON_TODO("Manage inputs??? up/down hierarchy, cut, copy, paste...");
    }
}

void HierarchyWindow::DrawSearchBar()
{
    static std::string filter;
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    if (ImGui::InputTextWithHint("##Filter", "Search GameObject", &filter, ImGuiInputTextFlags_AutoSelectAll))
    {
        Filter(filter);
    }

    ImGui::PopItemFlag();
}

HierarchyWindow::HierarchyStatus HierarchyWindow::DrawNodeTree(GameObject* rootGameObject)
{
    auto moduleScene = App->GetModule<ModuleScene>();
    std::stack<std::pair<GameObject*, bool>> stack;
    stack.push({ rootGameObject, false });

    while (!stack.empty())
    {
        auto& [gameObject, childrenVisited] = stack.top();
        stack.pop();

        if (childrenVisited)
        {
            ImGui::TreePop();
            continue;
        }

        std::ostringstream oss;
        oss << gameObject->GetName().c_str() << "###" << gameObject->GetUID();
        ImGui::PushID(oss.str().c_str());

        ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        auto hierarchyState = gameObject->GetHierarchySate();
        if (hierarchyState == HierarchyState::SELECTED)
        {
            treeFlags |= ImGuiTreeNodeFlags_Selected;
        }
        if (!gameObject->HasChildren())
        {
            treeFlags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (gameObject == moduleScene->GetRoot() || hierarchyState == HierarchyState::CHILD_SELECTED)
        {
            treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
        }

        bool nodeOpen = ImGui::TreeNodeEx(oss.str().c_str(), treeFlags);

        if (ImGui::IsItemClicked() && hierarchyState != HierarchyState::SELECTED)
        {
            moduleScene->SetSelectedGameObject(gameObject);
        }

        if (ImGui::BeginPopupContextItem("RightClickGameObject", ImGuiPopupFlags_MouseButtonRight)) 
        {
            if (ImGui::MenuItem("Create Empty child"))
            {
                moduleScene->CreateGameObject("Empty GameObject", gameObject);
            }
            if (IsMovable(gameObject))
            {
                DrawMoveObjectMenu(gameObject);
            }
            if (IsDeletable(gameObject) && DrawDeleteObjectMenu(gameObject))
            {
                ImGui::EndPopup();
                ImGui::PopID();
                if (nodeOpen)
                {
                    ImGui::TreePop();
                }
                continue;
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginDragDropSource())
        {
            UID uid = gameObject->GetUID();
            ImGui::SetDragDropPayload("HIERARCHY_GAMEOBJECT", &uid, sizeof(UID));
            ImGui::Text(gameObject->GetName().c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GAMEOBJECT"))
            {
                UID draggedGameObjectUID = *static_cast<UID*>(payload->Data);
                auto draggedGameObject = moduleScene->SearchGameObjectByUID(draggedGameObjectUID);
                if (draggedGameObject)
                {
                    draggedGameObject->SetParent(gameObject);
                    ImGui::EndDragDropTarget();
                    ImGui::PopID();
                    if (nodeOpen)
                    {
                        ImGui::TreePop();
                    }
                    continue;
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (nodeOpen)
        {
            stack.push({ gameObject, true });
            auto children = gameObject->GetChildren();
            for (int i = static_cast<int>(children.size()) - 1; i >= 0; i--)
            {
                stack.push({ children[i], false });
            }
        }
        ImGui::PopID();
    }
    return HierarchyStatus::SUCCESS;
}

HierarchyWindow::HierarchyStatus HierarchyWindow::DrawNodeTreeFiltered(GameObject* rootGameObject)
{
    auto moduleScene = App->GetModule<ModuleScene>();
    std::stack<std::pair<GameObject*, bool>> stack;
    stack.push({ rootGameObject, false });

    while (!stack.empty())
    {
        auto& [gameObject, childrenVisited] = stack.top();
        stack.pop();

        if (childrenVisited)
        {
            ImGui::TreePop();
            continue;
        }

        std::ostringstream oss;
        oss << gameObject->GetName().c_str() << "###" << gameObject->GetUID();
        ImGui::PushID(oss.str().c_str());

        ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        auto hierarchyState = gameObject->GetHierarchySate();
        if (hierarchyState == HierarchyState::SELECTED)
        {
            treeFlags |= ImGuiTreeNodeFlags_Selected;
        }
        if (!gameObject->HasChildren())
        {
            treeFlags |= ImGuiTreeNodeFlags_Leaf;
        }
        else
        {
            treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
        }

        bool nodeOpen = ImGui::TreeNodeEx(oss.str().c_str(), treeFlags);

        if (ImGui::IsItemClicked() && hierarchyState != HierarchyState::SELECTED)
        {
            moduleScene->SetSelectedGameObject(gameObject);
        }

        if (ImGui::BeginPopupContextItem("RightClickGameObject", ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::MenuItem("Create Empty child"))
            {
                moduleScene->CreateGameObject("Empty GameObject", gameObject);
            }
            if (IsMovable(gameObject))
            {
                DrawMoveObjectMenu(gameObject);
            }
            UID uid = gameObject->GetUID();
            if (IsDeletable(gameObject) && DrawDeleteObjectMenu(gameObject))
            {
                _uidFiltered.erase(uid);
                ImGui::EndPopup();
                ImGui::PopID();
                if (nodeOpen)
                {
                    ImGui::TreePop();
                }
                continue;
            }
            ImGui::EndPopup();
        }

        if (nodeOpen)
        {
            stack.push({ gameObject, true });
            auto children = gameObject->GetChildren();
            for (int i = static_cast<int>(children.size()) - 1; i >= 0; i--)
            {
                if (IsFiltered(children[i]->GetUID()))
                {
                    stack.push({ children[i], false });
                }
            }
        }
        ImGui::PopID();
    }
    return HierarchyStatus::SUCCESS;
}

void HierarchyWindow::DrawMoveObjectMenu(GameObject* gameObject)
{
    ImGui::Separator();

    if (ImGui::MenuItem("Move Up"))
    {
        gameObject->GetParent()->MoveChild(gameObject, -1);
    }

    if (ImGui::MenuItem("Move Down"))
    {
        gameObject->GetParent()->MoveChild(gameObject, 1);
    }
}

bool HierarchyWindow::DrawDeleteObjectMenu(GameObject* gameObject)
{
    ImGui::Separator();
    if (ImGui::MenuItem("Delete"))
    {
        auto moduleScene = App->GetModule<ModuleScene>();
        if (gameObject == moduleScene->GetSelectedGameObject())
        {
            moduleScene->SetSelectedGameObject(gameObject->GetParent());
        }
        moduleScene->RemoveGameObject(gameObject);
        return true;
    }
    return false;
}

bool HierarchyWindow::IsMovable(GameObject* gameObject)
{
    auto root = App->GetModule<ModuleScene>()->GetRoot();
    if (gameObject != root)
    {
        return true;
    }
    return false;
}

bool HierarchyWindow::IsDeletable(GameObject* gameObject)
{
    auto root = App->GetModule<ModuleScene>()->GetRoot();
    if (gameObject != root)
    {
        return true;
    }
    return false;
}

void HierarchyWindow::Filter(const std::string& filter)
{
    _uidFiltered.clear();

    if (filter.empty())
    {
        return;
    }

    std::string filterLower(filter.size(), '\0');
    std::transform(filter.begin(), filter.end(), filterLower.begin(), ::tolower);

    auto hierarchy = App->GetModule<ModuleScene>()->GetRoot()->GetAllDescendants();
    auto hierarchyFiltered = hierarchy | std::views::filter(
        [filterLower](GameObject* check)
        {
            auto& name = check->GetName();
            std::string nameLower(name.size(), '\0');
            std::transform(name.begin(), name.end(), nameLower.begin(), ::tolower);
            return nameLower.find(filterLower) != std::string::npos;
        });

    bool foundAnything = false;
    for (auto gameObjectFiltered : hierarchyFiltered)
    {
        foundAnything = true;
        for (auto ascendant : gameObjectFiltered->GetAllAscendants())
        {
            _uidFiltered.insert(ascendant->GetUID());
        }
    }
    if (!foundAnything)
    {
        _uidFiltered.insert(App->GetModule<ModuleScene>()->GetRoot()->GetUID());
    }
}