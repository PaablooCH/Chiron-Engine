#include "Pch.h"
#include "MainMenuWindow.h"

#include "Application.h"

#include "Modules/ModuleEditor.h"
#include "Modules/ModuleWindow.h"

#include "AboutWindow.h"

#include "DataModels/Window/Fonts/Font.h"

#include <ImGui/imgui.h>

MainMenuWindow::MainMenuWindow() : Window("Main Menu")
{
    _aboutWindow = std::make_unique<AboutWindow>();
}

MainMenuWindow::~MainMenuWindow()
{
}

void MainMenuWindow::Draw(const std::shared_ptr<CommandList>& commandList)
{
    if (ImGui::BeginMainMenuBar())
    {
        DrawFileMenu();
        DrawViewMenu();
        DrawHelpMenu();
    }
    ImGui::EndMainMenuBar();

    _aboutWindow->Draw();
}

void MainMenuWindow::DrawFileMenu()
{
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Exit"))
        {
            ::PostMessage(App->GetModule<ModuleWindow>()->GetWindowId(), WM_CLOSE, 0, 0);
        }
        ImGui::EndMenu();
    }
}

void MainMenuWindow::DrawViewMenu()
{
    if (ImGui::BeginMenu("View"))
    {
        auto& windowVector = App->GetModule<ModuleEditor>()->GetWindows();
        for (int i = 0; i < windowVector.size(); i++)
        {
            bool& showWindow = windowVector[i]->GetEnabled();
            ImGui::MenuItem(windowVector[i]->GetName().c_str(), NULL, &showWindow);
        }
        ImGui::EndMenu();
    }
}

void MainMenuWindow::DrawHelpMenu()
{
    if (ImGui::BeginMenu("Help"))
    {
        bool& showAbout = _aboutWindow->GetEnabled();
        ImGui::MenuItem(_aboutWindow->GetName().c_str(), NULL, &showAbout);
        Font::PushForkWebfont();
        if (ImGui::MenuItem(ICON_FK_GITHUB " GitHub Link"))
        {
            Chiron::Utils::OpenLink(REPOSITORY_URL);
        }
        Font::PopFont();
        ImGui::EndMenu();
    }
}