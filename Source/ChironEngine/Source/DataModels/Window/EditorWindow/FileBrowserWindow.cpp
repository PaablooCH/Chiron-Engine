#include "Pch.h"
#include "FileBrowserWindow.h"

FileBrowserWindow::FileBrowserWindow() : EditorWindow(ICON_FA_FOLDER " File Browser", ImGuiWindowFlags_AlwaysAutoResize)
{
}

FileBrowserWindow::~FileBrowserWindow()
{
}

void FileBrowserWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
}
