#include "Pch.h"
#include "MainMenuWindow.h"

#include "Application.h"

#include "Modules/ModuleEditor.h"
#include "Modules/ModuleWindow.h"

#include "AboutWindow.h"

#include <ImGui/imgui.h>

MainMenuWindow::MainMenuWindow(AboutWindow* aboutWindow) : Window("Main Menu"), _aboutWindow(aboutWindow)
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
        ImGui::MenuItem("About", NULL, &showAbout);
        if (ImGui::MenuItem("GitHub Link"))
        {
            Chiron::Utils::OpenLink(REPOSITORY_URL);
        }
        ImGui::EndMenu();
    }
}