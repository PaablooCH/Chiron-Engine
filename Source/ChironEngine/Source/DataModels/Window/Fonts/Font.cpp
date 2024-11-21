#include "Pch.h"
#include "Font.h"

#include "IconFontCppHeaders/IconsFontAwesome6.h"
#include "IconFontCppHeaders/IconsForkAwesome.h"

Font* Font::_instanceFont = nullptr;

void Font::DeleteInstance()
{
    delete _instanceFont;
}

void Font::SetDefaultFont()
{
    if (_instanceFont == nullptr)
    {
        GenerateInstance();
    }

    ImGuiIO& io = ImGui::GetIO();

    float baseFontSize = 16.0f;
    float iconFontSize = baseFontSize * 0.80f;

    _instanceFont->_robotoRegular = std::unique_ptr<ImFont>(io.Fonts->AddFontFromFileTTF("Fonts/Roboto/Roboto-Regular.ttf", baseFontSize));

    // merge in icons from Font Awesome
    static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphMinAdvanceX = iconFontSize;
    iconsConfig.GlyphOffset.y = -0.7f;
    io.Fonts->AddFontFromFileTTF("Fonts/FontAwesome/fa-solid-900.ttf", iconFontSize, &iconsConfig, iconsRanges);

    io.Fonts->Build();
    io.FontDefault = _instanceFont->_robotoRegular.get();
}

void Font::PushForkWebfont()
{
    if (_instanceFont == nullptr)
    {
        LOG_ERROR("Fonts instance is not created.");
        return;
    }

    ImGui::PushFont(_instanceFont->_forkWebfont.get());
    _instanceFont->_fontChanged = true;
}

bool Font::PopFont()
{
    if (_instanceFont->_fontChanged)
    {
        ImGui::PopFont();
        _instanceFont->_fontChanged = false;
        return true;
    }
    return false;
}

Font::Font() : _fontChanged(false)
{
}

Font::~Font()
{
}

void Font::GenerateInstance()
{
    if (_instanceFont == nullptr)
    {
        _instanceFont = new Font();

        // load fork awesome
        ImGuiIO& io = ImGui::GetIO();
        float baseFontSize = 16.0f;
        float iconFontSize = baseFontSize * 2.0f / 3.0f;

        _instanceFont->_forkWebfont = std::unique_ptr<ImFont>(io.Fonts->AddFontFromFileTTF("Fonts/Roboto/Roboto-Regular.ttf", baseFontSize));

        // merge in icons from Font Awesome
        static const ImWchar iconsRanges[] = { ICON_MIN_FK, ICON_MAX_16_FK, 0 };
        ImFontConfig iconsConfig;
        iconsConfig.MergeMode = true;
        iconsConfig.PixelSnapH = true;
        iconsConfig.GlyphMinAdvanceX = iconFontSize;
        io.Fonts->AddFontFromFileTTF("Fonts/ForkAwesome/forkawesome-webfont.ttf", iconFontSize, &iconsConfig, iconsRanges);

        io.Fonts->Build();
    }
}