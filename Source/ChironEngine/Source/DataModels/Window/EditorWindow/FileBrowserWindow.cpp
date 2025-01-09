#include "Pch.h"
#include "FileBrowserWindow.h"

#include "Modules/ModuleFileSystem.h"

#include "DataModels/FileSystem/Folder/Folder.h"

#include <sstream>

FileBrowserWindow::FileBrowserWindow() : EditorWindow(ICON_FA_FOLDER_TREE " File Browser", ImGuiWindowFlags_AlwaysAutoResize),
_currentPath("Assets")
{
    _rootFolder = std::make_unique<Folder>(_currentPath);
    SelectFolder(_rootFolder.get());

    GenerateFolders();
}

FileBrowserWindow::~FileBrowserWindow()
{
}

void FileBrowserWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
    if (ImGui::BeginChild("##FolderTreeChild", ImVec2(300, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders))
    {
        DrawFolderTree();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    
    ImGui::BeginGroup();
    if (_selectedFolder)
    {
        if (ImGui::BeginChild("##FolderInfoChild", ImVec2(0, 0), ImGuiChildFlags_Borders))
        {
            DrawFolderPath();

            ImGui::Separator();
            if (ImGui::BeginTable("##properties", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
            {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 2.0f);

                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndGroup();
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
            SelectFolder(folder);
        }

        if (ImGui::BeginPopupContextItem("RightClickFolder", ImGuiPopupFlags_MouseButtonRight)) 
        {
            if (ImGui::MenuItem("Create Folder"))
            {
                std::string newPath = folder->GetPath() + "New Folder";
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

void FileBrowserWindow::DrawFolderPath()
{
    for (int i = 0; i < _selectablePaths.size(); i++)
    {
        if (ImGui::Button(_selectablePaths[i].c_str()))
        {
            if (i != _selectablePaths.size() - 1)
            {
                std::vector<std::string> extracted(
                    std::make_move_iterator(_selectablePaths.begin()),
                    std::make_move_iterator(_selectablePaths.begin() + i + 1));
                SelectFolder(_rootFolder->FindFolder(extracted));
            }
        }
        if (i < _selectablePaths.size() - 1)
        {
            ImGui::SameLine();
            DrawButtonSubdirectories(i, _selectablePaths[i + 1]);
            ImGui::SameLine();
        }
    }
}

void FileBrowserWindow::DrawButtonSubdirectories(int iterator, const std::string& actualSubdirectory)
{
    std::string popupId = "SubdirectoriesMenu##" + std::to_string(iterator);
    std::string iconFolder = ICON_FA_GREATER_THAN;
    std::ostringstream oss;
    oss << iconFolder << "##" << iterator;
    if (ImGui::Button(oss.str().c_str()))
    {
        ImGui::OpenPopup(popupId.c_str());
    }
    if (ImGui::BeginPopup(popupId.c_str()))
    {
        if (static_cast<unsigned long long>(iterator) + 1 > _selectablePaths.size())
        {
            ImGui::EndPopup();
            return;
        }
        std::vector<std::string> extracted(
            _selectablePaths.begin(),
            _selectablePaths.begin() + iterator + 1);
        auto folder = _rootFolder->FindFolder(extracted);

        auto& subdirectories = folder->GetSubdirectories();
        for (int i = 0; i < subdirectories.size(); i++)
        {
            std::string name = subdirectories[i]->GetName();
            if (name.empty())
            {
                name = "Unnamed Folder";
            }
            std::string label = name + "##" + std::to_string(i);
            bool marked = name == actualSubdirectory;
            if (ImGui::MenuItem(label.c_str(), NULL, marked))
            {
                SelectFolder(subdirectories[i].get());
            }
        }
        ImGui::EndPopup();
    }
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

void FileBrowserWindow::SelectFolder(Folder* folder)
{
    _selectedFolder = folder;
    _selectablePaths = ModuleFileSystem::SplitPath(_selectedFolder->GetPath());
}