#include "Pch.h"
#include "FileBrowserWindow.h"

#include "Modules/ModuleFileSystem.h"

#include "DataModels/FileSystem/Folder/Folder.h"

#include <sstream>

FileBrowserWindow::FileBrowserWindow() : EditorWindow(ICON_FA_FOLDER_TREE " File Browser", ImGuiWindowFlags_AlwaysAutoResize),
_currentPath("Assets")
{
    _rootFolder = std::make_unique<Folder>(_currentPath);
    _selectedFolder = _rootFolder.get();

    GenerateFolders();
}

FileBrowserWindow::~FileBrowserWindow()
{
}

void FileBrowserWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
    DrawFolderTree();
}

void FileBrowserWindow::DrawFolderTree()
{
    std::stack<std::pair<Folder*, bool>> stack;
    stack.push({ _rootFolder.get(), false});

    while (!stack.empty())
    {
        auto& [folder, childrenVisited] = stack.top();
        stack.pop();

        if (childrenVisited)
        {
            ImGui::TreePop();
            continue;
        }

        std::ostringstream oss;
        std::string iconFolder = folder->GetOpen() ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER;
        oss << iconFolder << " " << folder->GetName().c_str() << "###" << folder->GetUID();
        ImGui::PushID(oss.str().c_str());

        ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (folder == _selectedFolder)
        {
            treeFlags |= ImGuiTreeNodeFlags_Selected;
        }
        if (!folder->HasSubdirectories())
        {
            if (folder->GetOpen())
            {
                folder->SetClosed();
            }
            treeFlags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (folder == _rootFolder.get())
        {
            treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
        }

        bool nodeOpen = ImGui::TreeNodeEx(oss.str().c_str(), treeFlags);

        if (ImGui::IsItemClicked() && folder != _selectedFolder)
        {
            _selectedFolder = folder;
        }

        if (ImGui::BeginPopupContextItem("RightClickFolder", ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::MenuItem("Create Folder"))
            {
                std::string newPath = folder->GetPath() + "New Folder";
                CHIRON_TODO("Problem with duplicated names");
                if (ModuleFileSystem::CreateUniqueDirectory(newPath))
                {
                    new Folder(newPath, folder);
                }
            }
            if (IsDeletable(folder) && DrawDeleteFolderMenu(folder))
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
            UID uid = folder->GetUID();
            ImGui::SetDragDropPayload("HIERARCHY_FOLDER", &uid, sizeof(uid));
            ImGui::Text(folder->GetName().c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_FOLDER"))
            {
                UID draggedUIDFolder = *static_cast<UID*>(payload->Data);
                auto draggedFolder = _rootFolder->FindFolder(draggedUIDFolder);
                if (draggedFolder)
                {
                    draggedFolder->SetParent(folder);
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
            if (!folder->GetOpen() && folder->HasSubdirectories())
            {
                folder->SetOpened();
            }
            stack.push({ folder, true });
            auto& subdirectories = folder->GetSubdirectories();
            for (int i = static_cast<int>(subdirectories.size()) - 1; i >= 0; i--)
            {
                Folder* subdirectory = subdirectories[i].get();
                stack.push({ subdirectory, false });
            }
        }
        else
        {
            if (folder->GetOpen())
            {
                folder->SetClosed();
            }
        }
        ImGui::PopID();
    }
}

bool FileBrowserWindow::DrawDeleteFolderMenu(Folder* folder)
{
    ImGui::Separator();
    if (ImGui::MenuItem("Delete Folder"))
    {
        auto parentFolder = folder->GetParent();
        if (folder == _selectedFolder)
        {
            _selectedFolder = parentFolder;
        }
        ModuleFileSystem::DeleteDirectory(folder->GetPath().c_str());
        delete parentFolder->UnlinkSubdirectory(folder);
        return true;
    }
    return false;
}

void FileBrowserWindow::GenerateFolders()
{
    std::vector<std::string> filesInLibPath = ModuleFileSystem::ListFilesWithPath((_currentPath + '/').c_str());
    std::queue<std::pair<std::string, Folder*>> filesToCheck;
    for (int i = 0; i < filesInLibPath.size(); i++)
    {
        filesToCheck.emplace(filesInLibPath[i], _rootFolder.get());
    }

    while (!filesToCheck.empty())
    {
        auto& pair = filesToCheck.front();
        std::string path = pair.first;
        filesToCheck.pop();
        if (ModuleFileSystem::IsDirectory(path.c_str()))
        {
            Folder* folder = new Folder(path, pair.second);
            path += "/";
            std::vector<std::string> filesInsideDirectory = ModuleFileSystem::ListFilesWithPath(path.c_str());

            for (const auto& file : filesInsideDirectory)
            {
                filesToCheck.emplace(file, folder);
            }
        }
    }
}