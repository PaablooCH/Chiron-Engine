#include "Pch.h"
#include "HierarchyWindow.h"

#include "Application.h"

#include "Modules/ModuleScene.h"

#include "DataModels/GameObject/GameObject.h"

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
        DrawNodeTree(root);
        CHIRON_TODO("Manage inputs???");
    }
}

void HierarchyWindow::DrawSearchBar()
{
    CHIRON_TODO("End Search Bar.");
    static char filter[256];
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    ImGui::InputTextWithHint("##Filter", "Search GameObject", filter, IM_ARRAYSIZE(filter));

    ImGui::PopItemFlag();
}

HierarchyWindow::HierarchyStatus HierarchyWindow::DrawNodeTree(GameObject* rootGameObject)
{
    auto moduleScene = App->GetModule<ModuleScene>();
    auto selected = moduleScene->GetSelectedGameObject();
    std::stack<std::pair<GameObject*, bool>> stack;
    stack.push({ rootGameObject, false });

    while (!stack.empty())
    {
        auto [gameObject, childrenVisited] = stack.top();
        stack.pop();

        if (childrenVisited)
        {
            ImGui::TreePop();
            continue;
        }

        char gameObjectLabel[160];
        sprintf_s(gameObjectLabel, "%s###%llu", gameObject->GetName().c_str(), gameObject->GetUID());
        ImGui::PushID(gameObjectLabel);

        ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (gameObject == selected)
        {
            treeFlags |= ImGuiTreeNodeFlags_Selected;
        }
        if (!gameObject->HasChildren())
        {
            treeFlags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (gameObject == moduleScene->GetRoot())
        {
            treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
        }

        bool nodeOpen = ImGui::TreeNodeEx(gameObjectLabel, treeFlags);

        if (ImGui::IsItemClicked() && gameObject != selected)
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
            ImGui::SetDragDropPayload("HIERARCHY", &uid, sizeof(UID));
            ImGui::Text(gameObject->GetName().c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY"))
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
                stack.push({ children[i], false});
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