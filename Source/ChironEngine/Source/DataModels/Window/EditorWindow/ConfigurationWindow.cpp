#include "Pch.h"
#include "ConfigurationWindow.h"

#include "Modules/ModuleFileSystem.h"

#include "SubWindows/Configuration/CameraSubWindow.h"
#include "SubWindows/Configuration/EngineStatisticsSubWindow.h"
#include "SubWindows/Configuration/HardwareSubWindow.h"

namespace
{
    const std::string configurationPath = "Settings/Configuration.conf";
}

ConfigurationWindow::ConfigurationWindow() : EditorWindow(ICON_FA_GEAR " Configuration")
{
    _subWindows.push_back(std::make_unique<EngineStatisticsSubWindow>());
    _subWindows.push_back(std::make_unique<CameraSubWindow>());
    _subWindows.push_back(std::make_unique<HardwareSubWindow>());

    LoadConfiguration();
}

ConfigurationWindow::~ConfigurationWindow()
{
    rapidjson::Document doc;
    Json json = Json(doc);
    for (auto& window : _subWindows)
    {
        Serializable* serialized = dynamic_cast<Serializable*>(window.get());
        if (serialized)
        {
            serialized->Save(json);
        }
    }

    auto buffer = json.ToBuffer();
    ModuleFileSystem::SaveFile(configurationPath.c_str(), buffer.GetString(), buffer.GetSize());
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

void ConfigurationWindow::LoadConfiguration()
{
    rapidjson::Document doc;
    Json json = Json(doc);
    if (ModuleFileSystem::LoadJson(configurationPath.c_str(), json) != -1)
    {
        for (auto& window : _subWindows)
        {
            Serializable* serialized = dynamic_cast<Serializable*>(window.get());
            if (serialized)
            {
                serialized->Load(json);
            }
        }
    }
}