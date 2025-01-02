#pragma once
#include "EditorWindow.h"

class FileBrowserWindow : public EditorWindow
{
public:
    FileBrowserWindow();
    ~FileBrowserWindow() override;
private:
    void DrawWindowContent(const std::shared_ptr<CommandList>& commandList) override;
};

