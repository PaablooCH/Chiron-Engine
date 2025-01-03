#include "Pch.h"
#include "ModuleEditor.h"

#include "Application.h"

#include "ModuleFileSystem.h"
#include "ModuleID3D12.h"
#include "ModuleWindow.h"

#include "DataModels/Window/AboutWindow.h"
#include "DataModels/Window/MainMenuWindow.h"
#include "DataModels/Window/EditorWindow/ConfigurationWindow.h"
#include "DataModels/Window/EditorWindow/ConsoleWindow.h"
#include "DataModels/Window/EditorWindow/FileBrowserWindow.h"
#include "DataModels/Window/EditorWindow/HierarchyWindow.h"
#include "DataModels/Window/EditorWindow/InspectorWindow.h"
#include "DataModels/Window/EditorWindow/SceneWindow.h"

#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/DescriptorAllocator/DescriptorAllocator.h"
#include "DataModels/DX12/DescriptorAllocator/DescriptorAllocation.h"
#include "DataModels/DX12/DescriptorAllocator/DescriptorAllocatorPage.h"
#include "DataModels/DX12/Resource/Texture.h"

#include "Defines/FileSystemDefine.h"

#include "DataModels/Window/Fonts/Font.h"

#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx12.h"
#include "ImGui/imgui_impl_win32.h"

#ifdef PROFILE
    #include "Optick/optick.h"
#endif // OPTICK

namespace
{
    const std::string windowsStatePath = "Settings/WindowsStates.conf";
}

ModuleEditor::ModuleEditor() : _dockFlags(0), _startDock(true)
{
}

ModuleEditor::~ModuleEditor()
{
}

bool ModuleEditor::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	// Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		// Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // Prevent mouse flickering
    ImGui::GetCurrentContext()->NavWindowingToggleLayer = false;

    ImGui_ImplWin32_Init(App->GetModule<ModuleWindow>()->GetWindowId());

    auto d3d12 = App->GetModule<ModuleID3D12>();
    _srvDescHeap = std::make_unique<DescriptorAllocation>(
        d3d12->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Allocate());
    ImGui_ImplDX12_Init(d3d12->GetDevice(), NUM_FRAMES_IN_FLIGHT, DXGI_FORMAT_R8G8B8A8_UNORM,
        _srvDescHeap->GetDescriptorAllocatorPage()->GetDescriptorHeap().Get(),
        _srvDescHeap->GetCPUDescriptorHandle(), _srvDescHeap->GetGPUDescriptorHandle());

    _windows.resize(static_cast<int>(WindowsType::SIZE));
    _windows[static_cast<int>(WindowsType::SCENE)] = std::make_unique<SceneWindow>();
    _windows[static_cast<int>(WindowsType::CONSOLE)] = std::make_unique<ConsoleWindow>();
    _windows[static_cast<int>(WindowsType::CONFIGURATION)] = std::make_unique<ConfigurationWindow>();
    _windows[static_cast<int>(WindowsType::FILE_BROWSER)] = std::make_unique<FileBrowserWindow>();
    _windows[static_cast<int>(WindowsType::HIERARCHY)] = std::make_unique<HierarchyWindow>();
    _windows[static_cast<int>(WindowsType::INSPECTOR)] = std::make_unique<InspectorWindow>();
    _mainMenu = std::make_unique<MainMenuWindow>();

    SetThemes();

    _startDock = !ModuleFileSystem::ExistsFile("imgui.ini");

    return true;
}

bool ModuleEditor::Start()
{
    Font::SetDefaultFont();
    ApplyTheme(_darkGreenStyle);
    SetStyle();

    LoadWindowsState();

    _dockFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
    _dockFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    return true;
}

bool ModuleEditor::CleanUp()
{
    SaveWindowsState();

    _mainMenu.reset();
    _windows.clear();
    _srvDescHeap.reset();
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    return true;
}

UpdateStatus ModuleEditor::PreUpdate()
{
#ifdef PROFILE
    OPTICK_CATEGORY("PreUpdateEditor", Optick::Category::UI);
#endif // DEBUG
    auto d3d12 = App->GetModule<ModuleID3D12>();

    _drawCommandList = d3d12->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
    FLOAT clearColor[] = { 0.4f, 0.4f, 0.4f, 1.0f }; // Set color

    // send the clear command into the list
    _drawCommandList->ClearRenderTargetView(d3d12->GetRenderBuffer(), clearColor, 0);

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleEditor::Update()
{
#ifdef PROFILE
    OPTICK_CATEGORY("UpdateEditor", Optick::Category::UI);
#endif // DEBUG
    auto d3d12 = App->GetModule<ModuleID3D12>();

    const ImGuiViewport* imGuiViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(imGuiViewport->WorkPos);
    ImGui::SetNextWindowSize(imGuiViewport->WorkSize);

    ImGui::Begin("DockSpace", nullptr, _dockFlags);
    ImGuiID dockSpaceId = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockSpaceId);
    if (_startDock)
    {
        _startDock = false;
        StartDock();
    }

    ImGui::End();

    _mainMenu->Draw();

    //ImGui::ShowDemoWindow();
    //ImGui::ShowMetricsWindow();

    for (std::unique_ptr<Window>& window : _windows)
    {
        window->Draw(_drawCommandList);
    }
    ImGui::Render();

    auto rtv = d3d12->GetRenderBuffer()->GetRenderTargetView().GetCPUDescriptorHandle();
    _drawCommandList->SetRenderTargets(1, &rtv, FALSE, nullptr);
    ID3D12DescriptorHeap* descriptorHeaps[] = {
        _srvDescHeap->GetDescriptorAllocatorPage()->GetDescriptorHeap().Get()
    };
    _drawCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), _drawCommandList->GetGraphicsCommandList().Get());

    _drawCommandList->TransitionBarrier(d3d12->GetRenderBuffer(), D3D12_RESOURCE_STATE_PRESENT);

    uint64_t fenceValue = d3d12->ExecuteCommandList(_drawCommandList);
    d3d12->SaveCurrentBufferFenceValue(fenceValue);

    return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleEditor::PostUpdate()
{
#ifdef PROFILE
    OPTICK_CATEGORY("PostUpdateEditor", Optick::Category::UI);
#endif // DEBUG
    App->GetModule<ModuleID3D12>()->PresentAndSwapBuffer();
    return UpdateStatus::UPDATE_CONTINUE;
}

void ModuleEditor::StartDock() const
{
    ImGuiID dockSpaceId = ImGui::GetID("DockSpace");

    ImGui::DockBuilderRemoveNode(dockSpaceId);
    ImGui::DockBuilderAddNode(dockSpaceId, _dockFlags | ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockSpaceId, ImGui::GetMainViewport()->Size);

    ImGuiID dockIdUp = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Up, 0.06f, nullptr, &dockSpaceId);
    ImGuiID dockIdRight = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Right, 0.27f, nullptr, &dockSpaceId);
    ImGuiID dockIdDown = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Down, 0.32f, nullptr, &dockSpaceId);
    ImGuiID dockIdLeft = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.22f, nullptr, &dockSpaceId);
    ImGui::DockBuilderDockWindow(ICON_FA_TERMINAL " Console", dockIdDown);
    ImGui::DockBuilderDockWindow(ICON_FA_FOLDER_TREE " File Browser", dockIdDown);
    //ImGui::DockBuilderDockWindow("State Machine Editor", dockIdDown);
    ImGui::DockBuilderDockWindow(ICON_FA_GEAR " Configuration", dockIdRight);
    //ImGui::DockBuilderDockWindow("Navigation", dockIdRight);
    //ImGui::DockBuilderDockWindow("Resources", dockIdRight);
    ImGui::DockBuilderDockWindow(ICON_FA_CIRCLE_INFO " Inspector", dockIdRight);
    //ImGui::DockBuilderDockWindow("Editor Control", dockIdUp);
    ImGui::DockBuilderDockWindow(ICON_FA_SITEMAP " Hierarchy", dockIdLeft);
    ImGui::DockBuilderDockWindow(ICON_FA_BOX_OPEN " Scene", dockSpaceId);
    ImGui::DockBuilderFinish(dockSpaceId);
}

void ModuleEditor::SaveWindowsState() const
{
    if (!ModuleFileSystem::IsDirectory(SETTINGS_FOLDER))
    {
        ModuleFileSystem::CreateDirectoryC(SETTINGS_FOLDER);
    }
    rapidjson::Document doc;
    Json json = Json(doc);

    for (auto& window : _windows)
    {
        json[window->GetName().c_str()] = window->GetEnabled();
    }
    auto aboutWindow = _mainMenu->GetAboutWindow();
    json[aboutWindow->GetName().c_str()] = aboutWindow->GetEnabled();

    auto buffer = json.ToBuffer();
    ModuleFileSystem::SaveFile(windowsStatePath.c_str(), buffer.GetString(), buffer.GetSize());
}

void ModuleEditor::LoadWindowsState()
{
    rapidjson::Document doc;
    Json json = Json(doc);

    if (ModuleFileSystem::LoadJson(windowsStatePath.c_str(), json) != -1)
    {
        for (auto& window : _windows)
        {
            window->SetEnabled(json[window->GetName().c_str()]);
        }
        auto aboutWindow = _mainMenu->GetAboutWindow();
        aboutWindow->SetEnabled(json[aboutWindow->GetName().c_str()]);
    }
}

void ModuleEditor::SetThemes()
{
    // ------------- COLORFULL STYLE ----------------------

    _colorfullStyle.Text = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    _colorfullStyle.TextDisabled = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    _colorfullStyle.WindowBg = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    _colorfullStyle.ChildBg = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    _colorfullStyle.PopupBg = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    _colorfullStyle.Border = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    _colorfullStyle.FrameBg = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    _colorfullStyle.FrameBgHovered = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    _colorfullStyle.FrameBgActive = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    _colorfullStyle.TitleBg = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
    _colorfullStyle.TitleBgActive = ImVec4(0.10f, 0.14f, 0.18f, 1.00f);
    _colorfullStyle.MenuBarBg = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    _colorfullStyle.ScrollbarBg = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    _colorfullStyle.ScrollbarGrab = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    _colorfullStyle.ScrollbarGrabHovered = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    _colorfullStyle.ScrollbarGrabActive = ImVec4(0.15f, 0.19f, 0.21f, 1.00f);
    _colorfullStyle.CheckMark = ImVec4(0.00f, 0.80f, 0.80f, 1.00f);
    _colorfullStyle.SliderGrab = ImVec4(0.00f, 0.64f, 0.92f, 1.00f);
    _colorfullStyle.SliderGrabActive = ImVec4(0.00f, 0.84f, 1.00f, 1.00f);
    _colorfullStyle.Button = ImVec4(0.20f, 0.52f, 0.67f, 1.00f);
    _colorfullStyle.ButtonHovered = ImVec4(0.25f, 0.62f, 0.82f, 1.00f);
    _colorfullStyle.ButtonActive = ImVec4(0.12f, 0.35f, 0.58f, 1.00f);
    _colorfullStyle.Header = ImVec4(0.15f, 0.30f, 0.60f, 1.00f);
    _colorfullStyle.HeaderHovered = ImVec4(0.20f, 0.40f, 0.72f, 1.00f);
    _colorfullStyle.HeaderActive = ImVec4(0.22f, 0.45f, 0.85f, 1.00f);
    _colorfullStyle.ResizeGrip = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    _colorfullStyle.ResizeGripHovered = ImVec4(0.12f, 0.48f, 0.88f, 1.00f);
    _colorfullStyle.ResizeGripActive = ImVec4(0.08f, 0.35f, 0.70f, 1.00f);

    // ------------- MINIMALIST STYLE ----------------------

    _minimalistStyle.Text = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    _minimalistStyle.TextDisabled = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    _minimalistStyle.WindowBg = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    _minimalistStyle.ChildBg = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    _minimalistStyle.PopupBg = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    _minimalistStyle.Border = ImVec4(0.40f, 0.40f, 0.40f, 0.50f);
    _minimalistStyle.FrameBg = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    _minimalistStyle.FrameBgHovered = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    _minimalistStyle.FrameBgActive = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    _minimalistStyle.TitleBg = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    _minimalistStyle.TitleBgActive = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    _minimalistStyle.MenuBarBg = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    _minimalistStyle.ScrollbarBg = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    _minimalistStyle.ScrollbarGrab = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    _minimalistStyle.ScrollbarGrabHovered = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    _minimalistStyle.ScrollbarGrabActive = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    _minimalistStyle.CheckMark = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    _minimalistStyle.SliderGrab = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    _minimalistStyle.SliderGrabActive = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    _minimalistStyle.Button = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    _minimalistStyle.ButtonHovered = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    _minimalistStyle.ButtonActive = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    _minimalistStyle.Header = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    _minimalistStyle.HeaderHovered = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    _minimalistStyle.HeaderActive = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    _minimalistStyle.ResizeGrip = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    _minimalistStyle.ResizeGripHovered = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    _minimalistStyle.ResizeGripActive = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

    // ------------- DARK GREEN STYLE ----------------------

    _darkGreenStyle.Text = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    _darkGreenStyle.TextDisabled = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    _darkGreenStyle.WindowBg = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    _darkGreenStyle.ChildBg = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    _darkGreenStyle.PopupBg = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    _darkGreenStyle.Border = ImVec4(0.19f, 0.20f, 0.20f, 1.00f);
    _darkGreenStyle.BorderShadow = ImVec4(0.25f, 0.25f, 0.25f, 0.00f);
    _darkGreenStyle.FrameBg = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    _darkGreenStyle.FrameBgHovered = ImVec4(0.17f, 0.17f, 0.17f, 0.70f);
    _darkGreenStyle.FrameBgActive = ImVec4(0.17f, 0.17f, 0.17f, 0.70f);
    _darkGreenStyle.TitleBg = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    _darkGreenStyle.TitleBgActive = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    _darkGreenStyle.TitleBgCollapsed = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    _darkGreenStyle.MenuBarBg = ImVec4(0.17f, 0.17f, 0.17f, 0.70f);
    _darkGreenStyle.ScrollbarBg = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    _darkGreenStyle.ScrollbarGrab = ImVec4(0.25f, 0.25f, 0.25f, 0.60f);
    _darkGreenStyle.ScrollbarGrabHovered = ImVec4(0.34f, 0.34f, 0.34f, 0.60f);
    _darkGreenStyle.ScrollbarGrabActive = ImVec4(0.41f, 0.41f, 0.41f, 0.60f);
    _darkGreenStyle.CheckMark = ImVec4(0.09f, 0.50f, 0.33f, 1.00f);
    _darkGreenStyle.SliderGrab = ImVec4(0.12f, 0.47f, 0.32f, 1.00f);
    _darkGreenStyle.SliderGrabActive = ImVec4(0.21f, 0.56f, 0.41f, 1.00f);
    _darkGreenStyle.Button = ImVec4(0.24f, 0.24f, 0.22f, 1.00f);
    _darkGreenStyle.ButtonHovered = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    _darkGreenStyle.ButtonActive = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
    _darkGreenStyle.Header = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
    _darkGreenStyle.HeaderHovered = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    _darkGreenStyle.HeaderActive = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
    _darkGreenStyle.Separator = ImVec4(0.34f, 0.34f, 0.34f, 0.60f);
    _darkGreenStyle.SeparatorHovered = ImVec4(0.34f, 0.34f, 0.34f, 0.60f);
    _darkGreenStyle.SeparatorActive = ImVec4(0.41f, 0.41f, 0.41f, 0.60f);
    _darkGreenStyle.ResizeGrip = ImVec4(0.11f, 0.30f, 0.22f, 1.00f);
    _darkGreenStyle.ResizeGripHovered = ImVec4(0.16f, 0.42f, 0.33f, 1.00f);
    _darkGreenStyle.ResizeGripActive = ImVec4(0.08f, 0.22f, 0.15f, 1.00f);
    _darkGreenStyle.TabHovered = ImVec4(0.16f, 0.42f, 0.33f, 1.00f);
    _darkGreenStyle.Tab = ImVec4(0.11f, 0.30f, 0.22f, 1.00f);
    _darkGreenStyle.TabSelected = ImVec4(0.09f, 0.50f, 0.33f, 1.00f);
    _darkGreenStyle.TabSelectedOverline = ImVec4(0.25f, 0.25f, 0.25f, 0.60f);
    _darkGreenStyle.TabDimmed = ImVec4(0.11f, 0.30f, 0.22f, 1.00f);
    _darkGreenStyle.TabDimmedSelected = ImVec4(0.09f, 0.50f, 0.33f, 1.00f);
    _darkGreenStyle.TabDimmedSelectedOverline = ImVec4(0.25f, 0.25f, 0.25f, 0.60f);
    _darkGreenStyle.DockingPreview = ImVec4(0.11f, 0.30f, 0.22f, 1.00f);
    _darkGreenStyle.DockingEmptyBg = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    _darkGreenStyle.PlotLines = ImVec4(0.12f, 0.47f, 0.32f, 1.00f);
    _darkGreenStyle.PlotLinesHovered = ImVec4(0.18f, 0.59f, 0.42f, 1.00f);
    _darkGreenStyle.PlotHistogram = ImVec4(0.12f, 0.47f, 0.32f, 1.00f);
    _darkGreenStyle.PlotHistogramHovered = ImVec4(0.18f, 0.59f, 0.42f, 1.00f);
    _darkGreenStyle.TableHeaderBg = ImVec4(0.16f, 0.43f, 0.31f, 1.00f);
    _darkGreenStyle.TableBorderStrong = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    _darkGreenStyle.TableBorderLight = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    _darkGreenStyle.TableRowBg = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    _darkGreenStyle.TableRowBgAlt = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    _darkGreenStyle.TextLink = ImVec4(0.22f, 0.70f, 0.51f, 1.00f);
    _darkGreenStyle.TextSelectedBg = ImVec4(0.23f, 0.59f, 0.45f, 1.00f);
    _darkGreenStyle.DragDropTarget = ImVec4(0.11f, 0.30f, 0.22f, 1.00f);
    _darkGreenStyle.NavHighlight = ImVec4(0.22f, 0.63f, 0.46f, 1.00f);
    _darkGreenStyle.NavWindowingHighlight = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    _darkGreenStyle.NavWindowingDimBg = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    _darkGreenStyle.ModalWindowDimBg = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ModuleEditor::ApplyTheme(const ThemeColors& theme)
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = theme.Text;
    colors[ImGuiCol_TextDisabled] = theme.TextDisabled;
    colors[ImGuiCol_WindowBg] = theme.WindowBg;
    colors[ImGuiCol_ChildBg] = theme.ChildBg;
    colors[ImGuiCol_PopupBg] = theme.PopupBg;
    colors[ImGuiCol_Border] = theme.Border;
    colors[ImGuiCol_BorderShadow] = theme.BorderShadow;
    colors[ImGuiCol_FrameBg] = theme.FrameBg;
    colors[ImGuiCol_FrameBgHovered] = theme.FrameBgHovered;
    colors[ImGuiCol_FrameBgActive] = theme.FrameBgActive;
    colors[ImGuiCol_TitleBg] = theme.TitleBg;
    colors[ImGuiCol_TitleBgActive] = theme.TitleBgActive;
    colors[ImGuiCol_TitleBgCollapsed] = theme.TitleBgCollapsed;
    colors[ImGuiCol_MenuBarBg] = theme.MenuBarBg;
    colors[ImGuiCol_ScrollbarBg] = theme.ScrollbarBg;
    colors[ImGuiCol_ScrollbarGrab] = theme.ScrollbarGrab;
    colors[ImGuiCol_ScrollbarGrabHovered] = theme.ScrollbarGrabHovered;
    colors[ImGuiCol_ScrollbarGrabActive] = theme.ScrollbarGrabActive;
    colors[ImGuiCol_CheckMark] = theme.CheckMark;
    colors[ImGuiCol_SliderGrab] = theme.SliderGrab;
    colors[ImGuiCol_SliderGrabActive] = theme.SliderGrabActive;
    colors[ImGuiCol_Button] = theme.Button;
    colors[ImGuiCol_ButtonHovered] = theme.ButtonHovered;
    colors[ImGuiCol_ButtonActive] = theme.ButtonActive;
    colors[ImGuiCol_Header] = theme.Header;
    colors[ImGuiCol_HeaderHovered] = theme.HeaderHovered;
    colors[ImGuiCol_HeaderActive] = theme.HeaderActive;
    colors[ImGuiCol_Separator] = theme.Separator;
    colors[ImGuiCol_SeparatorHovered] = theme.SeparatorHovered;
    colors[ImGuiCol_SeparatorActive] = theme.SeparatorActive;
    colors[ImGuiCol_ResizeGrip] = theme.ResizeGrip;
    colors[ImGuiCol_ResizeGripHovered] = theme.ResizeGripHovered;
    colors[ImGuiCol_ResizeGripActive] = theme.ResizeGripActive;
    colors[ImGuiCol_TabHovered] = theme.TabHovered;
    colors[ImGuiCol_Tab] = theme.Tab;
    colors[ImGuiCol_TabSelected] = theme.TabSelected;
    colors[ImGuiCol_TabSelectedOverline] = theme.TabSelectedOverline;
    colors[ImGuiCol_TabDimmed] = theme.TabDimmed;
    colors[ImGuiCol_TabDimmedSelected] = theme.TabDimmedSelected;
    colors[ImGuiCol_TabDimmedSelectedOverline] = theme.TabDimmedSelectedOverline;
    colors[ImGuiCol_DockingPreview] = theme.DockingPreview;
    colors[ImGuiCol_DockingEmptyBg] = theme.DockingEmptyBg;
    colors[ImGuiCol_PlotLines] = theme.PlotLines;
    colors[ImGuiCol_PlotLinesHovered] = theme.PlotLinesHovered;
    colors[ImGuiCol_PlotHistogram] = theme.PlotHistogram;
    colors[ImGuiCol_PlotHistogramHovered] = theme.PlotHistogramHovered;
    colors[ImGuiCol_TableHeaderBg] = theme.TableHeaderBg;
    colors[ImGuiCol_TableBorderStrong] = theme.TableBorderStrong;
    colors[ImGuiCol_TableBorderLight] = theme.TableBorderLight;
    colors[ImGuiCol_TableRowBg] = theme.TableRowBg;
    colors[ImGuiCol_TableRowBgAlt] = theme.TableRowBgAlt;
    colors[ImGuiCol_TextLink] = theme.TextLink;
    colors[ImGuiCol_TextSelectedBg] = theme.TextSelectedBg;
    colors[ImGuiCol_DragDropTarget] = theme.DragDropTarget;
    colors[ImGuiCol_NavHighlight] = theme.NavHighlight;
    colors[ImGuiCol_NavWindowingHighlight] = theme.NavWindowingHighlight;
    colors[ImGuiCol_NavWindowingDimBg] = theme.NavWindowingDimBg;
    colors[ImGuiCol_ModalWindowDimBg] = theme.ModalWindowDimBg;
}

void ModuleEditor::SetStyle()
{
    ImGui::GetStyle().WindowPadding = ImVec2(8.f, 8.f);
    ImGui::GetStyle().FramePadding = ImVec2(4.f, 3.f);
    ImGui::GetStyle().ItemSpacing = ImVec2(8.f, 4.f);
    ImGui::GetStyle().ItemInnerSpacing = ImVec2(4.f, 4.f);
    ImGui::GetStyle().TouchExtraPadding = ImVec2(0.f, 0.f);
    ImGui::GetStyle().IndentSpacing = 21.f;
    ImGui::GetStyle().ScrollbarSize = 14.f;
    ImGui::GetStyle().GrabMinSize = 12.f;

    ImGui::GetStyle().WindowBorderSize = 1.f;
    ImGui::GetStyle().ChildBorderSize = 1.f;
    ImGui::GetStyle().PopupBorderSize = 1.f;
    ImGui::GetStyle().FrameBorderSize = 0.f;
    ImGui::GetStyle().TabBorderSize = 0.f;
    ImGui::GetStyle().TabBarBorderSize = 1.f;
    ImGui::GetStyle().TabBarOverlineSize = 2.f;

    ImGui::GetStyle().WindowRounding = 7.f;
    ImGui::GetStyle().ChildRounding = 6.f;
    ImGui::GetStyle().FrameRounding = 6.f;
    ImGui::GetStyle().PopupRounding = 6.f;
    ImGui::GetStyle().ScrollbarRounding = 9.f;
    ImGui::GetStyle().GrabRounding = 6.f;
    ImGui::GetStyle().TabRounding = 6.f;
}