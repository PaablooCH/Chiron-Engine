#include "Pch.h"
#include "ConfigurationWindow.h"

#include "SubWindows/Configuration/CameraSubWindow.h"
#include "SubWindows/Configuration/EngineStatisticsSubWindow.h"
#include "SubWindows/Configuration/HardwareSubWindow.h"

ConfigurationWindow::ConfigurationWindow() : EditorWindow(ICON_FA_GEAR " Configuration")
{
    _subWindows.push_back(std::make_unique<EngineStatisticsSubWindow>());
    _subWindows.push_back(std::make_unique<CameraSubWindow>());
    _subWindows.push_back(std::make_unique<HardwareSubWindow>());
}

ConfigurationWindow::~ConfigurationWindow()
{
}

void ConfigurationWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
    for (int i = 0; i < _subWindows.size(); i++)
    {
        _subWindows[i]->Draw(commandList);
        if (i < _subWindows.size() - 1)
        {
            ImGui::Separator();
        }
    }
}