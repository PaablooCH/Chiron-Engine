#pragma once

#include "ImGui/imgui.h"

class Font
{
public:
    static void DeleteInstance();
    static void SetDefaultFont();

    static void PushForkWebfont();

    static bool PopFont();

private:
    Font();
    ~Font();

    static Font* _instanceFont;

    static void GenerateInstance();

    std::unique_ptr<ImFont> _robotoRegular;
    std::unique_ptr<ImFont> _forkWebfont;

    bool _fontChanged;
};
