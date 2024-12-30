#pragma once
#include "Window.h"

class AboutWindow;

class MainMenuWindow : public Window
{
public:
    MainMenuWindow();
    ~MainMenuWindow() override;

    void Draw(const std::shared_ptr<CommandList>& commandList = nullptr) override;

    // ------------- GETTERS ----------------------

    inline AboutWindow* GetAboutWindow() const;

private:
    void DrawFileMenu();
    void DrawViewMenu();
    void DrawHelpMenu();

private:
    std::unique_ptr<AboutWindow> _aboutWindow;
};

inline AboutWindow* MainMenuWindow::GetAboutWindow() const
{
    return _aboutWindow.get();
}