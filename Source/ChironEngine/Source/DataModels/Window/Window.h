#pragma once

#include "IconFontCppHeaders/IconsFontAwesome6.h"
#include "IconFontCppHeaders/IconsForkAwesome.h"

class CommandList;

class Window
{
public:
    virtual ~Window();

    virtual void Draw(const std::shared_ptr<CommandList>& commandList) = 0;

    // ------------- GETTERS ----------------------

    inline const std::string& GetName() const;
    inline bool& GetEnabled();

    // ------------- GETTERS ----------------------

    inline void SetEnabled(bool enabled);

protected:
    Window(const std::string& name);

protected:
    std::string _name;
    bool _enabled;
};

inline const std::string& Window::GetName() const
{
    return _name;
}

inline bool& Window::GetEnabled()
{
    return _enabled;
}

inline void Window::SetEnabled(bool enabled)
{
    _enabled = enabled;
}