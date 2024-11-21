#include "Pch.h"
#include "AboutWindow.h"

#include <ImGui/imgui.h>

AboutWindow::AboutWindow() : Window(ICON_FA_CIRCLE_QUESTION " About"), _first(true)
{
}

AboutWindow::~AboutWindow()
{
}

void AboutWindow::Draw(const std::shared_ptr<CommandList>& commandList)
{
    if (_enabled)
    {
        if (_first)
        {
            _first = false;
            ImVec2 screenSize = ImGui::GetIO().DisplaySize;

            // Define el tamaño de la ventana que deseas
            ImVec2 windowSize(600, 300); // Por ejemplo, 400x300 píxeles

            // Calcula la posición para centrar la ventana
            ImVec2 windowPos((screenSize.x - windowSize.x) * 0.5f, (screenSize.y - windowSize.y) * 0.5f);

            ImGui::SetNextWindowPos(windowPos);
            ImGui::SetNextWindowSize(windowSize);
        }

        if (ImGui::Begin(_name.c_str(), &_enabled, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoNav))
        {
            ImGui::Text("CHIRON ENGINE v0.1");
            ImGui::Text("3D Game engine made with C++ & DirectX12, created and developed by");
            ImGui::SameLine();
            ImGui::TextLinkOpenURL("Pablo CH", REPOSITORY_URL);
            ImGui::SameLine(0.f, 0.f);
            ImGui::Text(".");
            ImGui::Separator();
            ImGui::Text("Copyright 2024 Pablo Cebollada Hernandez");
            ImGui::TextWrapped("Licensed under the Apache License, Version 2.0 (the \"License\"); you may not use this file except in compliance with the License.");
            ImGui::TextWrapped("Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.");
        }
        ImGui::End();
    }
}