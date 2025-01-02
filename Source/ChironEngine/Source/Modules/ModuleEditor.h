#pragma once
#include "Module.h"
#include "Structs/ThemeColors.h"

class CommandList;
class MainMenuWindow;
class Window;
class DescriptorAllocation;

class ModuleEditor : public Module
{
public:
    ModuleEditor();
    ~ModuleEditor() override;

    bool Init() override;
    bool Start() override;
    bool CleanUp() override;

    UpdateStatus PreUpdate() override;
    UpdateStatus Update() override;
    UpdateStatus PostUpdate() override;

    inline const std::vector<std::unique_ptr<Window>>& GetWindows() const;

private:
    void StartDock() const;
    void SaveWindowsState() const;
    void LoadWindowsState();

    void SetThemes();
    void ApplyTheme(const ThemeColors& theme);

    void SetStyle();

private:
    enum class WindowsType
    {
        CONFIGURATION,
        CONSOLE,
        FILE_BROWSER,
        HIERARCHY,
        INSPECTOR,
        SCENE,
        SIZE
    };

    std::vector<std::unique_ptr<Window>> _windows;
    std::unique_ptr<MainMenuWindow> _mainMenu;

    ThemeColors _colorfullStyle;
    ThemeColors _minimalistStyle;
    ThemeColors _darkGreenStyle;

    std::unique_ptr<DescriptorAllocation> _srvDescHeap;

    std::shared_ptr<CommandList> _drawCommandList;

    ImGuiWindowFlags _dockFlags;

    bool _startDock;
};

inline const std::vector<std::unique_ptr<Window>>& ModuleEditor::GetWindows() const
{
    return _windows;
}