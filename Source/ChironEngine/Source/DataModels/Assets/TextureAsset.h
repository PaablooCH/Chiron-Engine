#pragma once
#include "Asset.h"

class Texture;

enum class TextureType
{
    ALBEDO,
    DIFFUSE = ALBEDO,
    SPECULAR,
    METALLIC = SPECULAR,
    NORMAL_MAP,
    EMISSIVE,
    OCCLUSION,
    HDR,
    DEPTH,
    RENDER_TARGET
};

enum TexConfig
{
    isBottomLeft = 0x00000001,         // Where start when reading the textures
};

enum TexConversionFlags
{
    kSRGB = 0x00000001,             // Texture contains sRGB colors
    kPreserveAlpha = 0x00000002,    // Keep four channels
    kNormalMap = 0x00000004,        // Texture contains normals
    kBumpToNormal = 0X00000008,     // Generate a normal map from a bump map
    kDefaultBC = 0X00000010,        // Apply standard block compression (BC1-5)
    kQualityBC = 0X00000020,        // Apply quality block compression (BC6H/7)
    kFlipVertical = 0X00000040,
    kFlipHorizontal = 0X00000080,
};

class TextureAsset : public Asset
{
public:
    TextureAsset(TextureType type);
    TextureAsset(TextureType type, UID uid, const std::string& assetPath, const std::string& libraryPath);
    TextureAsset(UID uid, const std::string& assetPath, const std::string& libraryPath);
    ~TextureAsset() override;

    inline void AddConfigFlags(unsigned int flags);
    inline void RemoveConfigFlags(unsigned int flags);
    void AddConversionFlags(unsigned int flags);
    void RemoveConversionFlags(unsigned int flags);

    // ------------- GETTERS ----------------------

    inline std::shared_ptr<Texture> GetTexture();
    inline TextureType GetTextureType() const;
    inline unsigned int GetConfigFlags() const;
    inline bool GetConversionFlag(TexConversionFlags flag) const;
    inline unsigned int GetConversionFlags() const;
    std::string GetLibraryDDSPath() const;

    // ------------- SETTERS ----------------------

    void SetTexture(std::shared_ptr<Texture>& newTexture);
    inline void SetTextureType(TextureType newType);

private:
    bool InternalLoad() override;
    bool InternalUnload() override;

private:
    std::shared_ptr<Texture> _texture;

    TextureType _type;
    unsigned int _texConfigFlags;
    unsigned int _texConversionFlags;
};

void TextureAsset::AddConfigFlags(unsigned int flags)
{
    _texConfigFlags |= flags;
}

void TextureAsset::RemoveConfigFlags(unsigned int flags)
{
    _texConfigFlags &= ~flags;
}

inline std::shared_ptr<Texture> TextureAsset::GetTexture()
{
    if (!IsValid())
    {
        Load();
    }
    return _texture;
}

inline TextureType TextureAsset::GetTextureType() const
{
    return _type;
}

inline unsigned int TextureAsset::GetConfigFlags() const
{
    return _texConfigFlags;
}

inline bool TextureAsset::GetConversionFlag(TexConversionFlags flag) const
{
    return (_texConversionFlags & flag) != 0;
}

inline unsigned int TextureAsset::GetConversionFlags() const
{
    return _texConversionFlags;
}

inline void TextureAsset::SetTextureType(TextureType newType)
{
    _type = newType;
    _texConversionFlags = 0;
    if (_type == TextureType::ALBEDO)
    {
        _texConversionFlags |= kSRGB;
        _texConversionFlags |= kPreserveAlpha;
    }
    else if (_type == TextureType::NORMAL_MAP)
    {
        _texConversionFlags |= kNormalMap;
    }
    else if (_type == TextureType::HDR)
    {
        _texConversionFlags |= kQualityBC;
    }
    _texConversionFlags |= kDefaultBC;
}

inline void TextureAsset::SetTextureType(TextureType newType)
{
    _type = newType;
}
