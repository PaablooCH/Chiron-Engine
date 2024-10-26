#include "Pch.h"
#include "HierarchyWindow.h"

#include "Application.h"

#include "Modules/ModuleScene.h"

#include "DataModels/GameObject/GameObject.h"

HierarchyWindow::HierarchyWindow() : EditorWindow("Hierarchy")
{
    _flags |= ImGuiWindowFlags_AlwaysAutoResize;
}

HierarchyWindow::~HierarchyWindow()
{
}

void HierarchyWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
    DrawSearchBar();
    
    ImGui::Separator();

    auto root = App->GetModule<ModuleScene>()->GetRoot();
    if (root)
    {
        DrawNodeTree(root);
        CHIRON_TODO("Manage inputs???");
    }
}

void HierarchyWindow::DrawSearchBar()
{
    static char filter[256];
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    ImGui::InputTextWithHint("##Filter", "Search GameObject", filter, IM_ARRAYSIZE(filter));

    ImGui::PopItemFlag();
}

HierarchyWindow::HierarchyStatus HierarchyWindow::DrawNodeTree(GameObject* gameObject)
{
    auto moduleScene = App->GetModule<ModuleScene>();
    auto selected = moduleScene->GetSelectedGameObject();

    char gameObjectLabel[160];
    sprintf_s(gameObjectLabel, "%s###%p", gameObject->GetName().c_str(), gameObject);
    ImGui::PushID(gameObjectLabel);

    ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_None;
    treeFlags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
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
    
    if (gameObject != selected && (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || 
        ImGui::IsMouseReleased(ImGuiMouseButton_Right)) &&
        ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
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

        if (IsDeletable(gameObject))
        {
            if (DrawDeleteObjectMenu(gameObject))
            {
                ImGui::EndPopup();
                ImGui::PopID();
                if (nodeOpen)
                {
                    ImGui::TreePop();
                }
                return HierarchyStatus::HAS_CHANGES;
            }
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
                CHIRON_TODO("When Quatree/Octree is implemented if the dragged is in/out selected add/remove it from static/dynamic vectors");

                draggedGameObject->SetParent(gameObject);
                ImGui::EndDragDropTarget();
                ImGui::PopID();
                if (nodeOpen)
                {
                    ImGui::TreePop();
                }
                return HierarchyStatus::HAS_CHANGES;
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (nodeOpen)
    {
        for (auto child : gameObject->GetChildren())
        {
            if (DrawNodeTree(child) == HierarchyStatus::HAS_CHANGES)
            {
                ImGui::PopID();
                if (nodeOpen)
                {
                    ImGui::TreePop();
                }
                return HierarchyStatus::HAS_CHANGES;
            }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
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
