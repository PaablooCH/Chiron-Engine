#pragma once
#include "EditorWindow.h"

class ConfigurationWindow : public EditorWindow
{
public:
    ConfigurationWindow();
    ~ConfigurationWindow() override;

private:
    void DrawWindowContent(const std::shared_ptr<CommandList>& commandList) override;

    void LoadConfiguration();

private:
    std::vector<std::unique_ptr<Window>> _subWindows;
};
