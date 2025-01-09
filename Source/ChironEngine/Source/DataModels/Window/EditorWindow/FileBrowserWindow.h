#pragma once
#include "EditorWindow.h"

class Folder;

class FileBrowserWindow : public EditorWindow
{
public:
    FileBrowserWindow();
    ~FileBrowserWindow() override;
private:
    void DrawWindowContent(const std::shared_ptr<CommandList>& commandList) override;
    void DrawFolderTree();
    bool DrawDeleteFolderMenu(Folder* folder);
    void DrawFolderPath();
    void DrawButtonSubdirectories(int i, const std::string& actualSubdirectory);

    void GenerateFolders();
    void SelectFolder(Folder* folder);

    inline bool IsDeletable(Folder* folder) const;
private:
    std::string _currentPath;

    std::unique_ptr<Folder> _rootFolder;
    Folder* _selectedFolder;

    std::vector<std::string> _selectablePaths;
};

inline bool FileBrowserWindow::IsDeletable(Folder* folder) const
{
    return folder != _rootFolder.get();
}