#include "Pch.h"
#include "TransformComponentWindow.h"

#include "DataModels/GameObject/GameObject.h"
#include "DataModels/Components/TransformComponent.h"

namespace
{
    ImVec4 red = ImVec4(0.753f, 0.224f, 0.169f, 1.0f);
    ImVec4 redHover = ImVec4(0.749f, 0.267f, 0.224f, 1.0f);
    ImVec4 redActive = ImVec4(0.649f, 0.217f, 0.174f, 1.0f);
    ImVec4 green = ImVec4(0.510f, 0.655f, 0.180f, 1.0f);
    ImVec4 greenHover = ImVec4(0.6f, 0.749f, 0.224f, 1.0f);
    ImVec4 greenActive = ImVec4(0.5f, 0.649f, 0.174f, 1.0f);
    ImVec4 blue = ImVec4(0.169f, 0.349f, 0.753f, 1.0f);
    ImVec4 blueHover = ImVec4(0.224f, 0.392f, 0.749f, 1.0f);
    ImVec4 blueActive = ImVec4(0.174f, 0.292f, 0.649f, 1.0f);
    ImVec4 greyHeader = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
    ImVec4 greyActive = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    ImVec4 greyHover = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
}

TransformComponentWindow::TransformComponentWindow(TransformComponent* component) : 
    ComponentWindow(ICON_FA_UP_DOWN_LEFT_RIGHT " Transform", component, true, true), _dragSpeed(0.03f)
{
}

TransformComponentWindow::~TransformComponentWindow()
{
}

void TransformComponentWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
    auto transformComponent = static_cast<TransformComponent*>(_component);
    if (transformComponent)
    {
        CHIRON_TODO("Implement bb");
        DrawTransforms();
    }
}

void TransformComponentWindow::DrawTransforms()
{
    auto transformComponent = static_cast<TransformComponent*>(_component);

    bool posModified = false;
    bool rotModified = false;
    bool scaModified = false;

    Vector3 localPos = transformComponent->GetLocalPos();
    Vector3 localRot = transformComponent->GetLocalRotXYZ();
    Vector3 localSca = transformComponent->GetLocalSca();

    bool uniformScale = transformComponent->GetUniformScale();
    Axis modifiedAxis = Axis::NONE;

    if (ImGui::BeginTable("TransformTable", 2, ImGuiTableFlags_BordersOuter))
    {
        ImGui::TableSetupColumn("###transFirstCol", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("###transSecondCol", ImGuiTableColumnFlags_WidthStretch);

        // ------------- POSITION ----------------------

        ImGui::TableNextColumn();
        ImGui::Dummy(ImVec2(0.f, 5.f));
        ImGui::TableNextColumn();

        ImGui::TableNextColumn();
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Text("Pos");

        ImGui::TableNextColumn();
        float itemWidth = (ImGui::GetContentRegionAvail().x - 20) / 3;
        ImGui::PushStyleColor(ImGuiCol_FrameBg, red);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, redHover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, redActive);
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::DragFloat("##XTrans", &localPos.x, _dragSpeed, std::numeric_limits<float>::min(), 
            std::numeric_limits<float>::max()))
        {
            posModified = true;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, green);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, greenHover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, greenActive);
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::DragFloat("##YTrans", &localPos.y, _dragSpeed, std::numeric_limits<float>::min(), 
            std::numeric_limits<float>::max()))
        {
            posModified = true;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, blue);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, blueHover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, blueActive);
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::DragFloat("##ZTrans", &localPos.z, _dragSpeed, std::numeric_limits<float>::min(), 
            std::numeric_limits<float>::max()))
        {
            posModified = true;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        // ------------- ROTATION ----------------------

        ImGui::TableNextColumn();
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Text("Rot");
        
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, red);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, redHover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, redActive);
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::DragFloat("##XRot", &localRot.x, _dragSpeed, std::numeric_limits<float>::min(), 
            std::numeric_limits<float>::max()))
        {
            rotModified = true;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, green);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, greenHover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, greenActive);
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::DragFloat("##YRot", &localRot.y, _dragSpeed, std::numeric_limits<float>::min(), 
            std::numeric_limits<float>::max()))
        {
            rotModified = true;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, blue);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, blueHover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, blueActive);
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::DragFloat("##ZRot", &localRot.z, _dragSpeed, std::numeric_limits<float>::min(), 
            std::numeric_limits<float>::max()))
        {
            rotModified = true;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        // ------------- SCALE ----------------------

        ImGui::TableNextColumn();
        ImGui::Dummy(ImVec2(0, 1));
        ImGui::Text("Sca");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Header, greyHeader);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, greyHover);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, greyActive);
        std::string uniformLabel = uniformScale ? ICON_FA_LINK : ICON_FA_LINK_SLASH;
        if (ImGui::Selectable(uniformLabel.c_str(), &uniformScale))
        {
            transformComponent->SetUniformScale(uniformScale);
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            std::string uniformToolTipLabel = uniformScale ? "Disable constrained proportions" : "Enable constrained proportions";
            ImGui::TextUnformatted(uniformToolTipLabel.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }

        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, red);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, redHover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, redActive);
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::DragFloat("##XSca", &localSca.x, _dragSpeed, 0.001f, std::numeric_limits<float>::max()))
        {
            scaModified = true;
            modifiedAxis = Axis::X;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, green);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, greenHover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, greenActive);
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::DragFloat("##YSca", &localSca.y, _dragSpeed, 0.001f, std::numeric_limits<float>::max()))
        {
            scaModified = true;
            modifiedAxis = Axis::Y;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, blue);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, blueHover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, blueActive);
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::DragFloat("##ZSca", &localSca.z, _dragSpeed, 0.001f, std::numeric_limits<float>::max()))
        {
            scaModified = true;
            modifiedAxis = Axis::Z;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();
        ImGui::Dummy(ImVec2(0.f, 5.f));
        ImGui::TableNextColumn();

        ImGui::EndTable();
    }

    if (posModified)
    {
        transformComponent->SetLocalPos(localPos);
    }

    if (rotModified)
    {
        transformComponent->SetLocalRot(localRot);
    }

    if (scaModified)
    {
        if (uniformScale)
        {
            transformComponent->SetScaleUniform(localSca, modifiedAxis);
        }
        else
        {
            transformComponent->SetLocalSca(localSca);
        }
    }

    if (posModified || rotModified || scaModified)
    {
        transformComponent->UpdateMatrices();
    }
}
