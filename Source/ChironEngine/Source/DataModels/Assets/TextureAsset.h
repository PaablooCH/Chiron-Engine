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
    ~TextureAsset() override;

    inline void AddConfigFlags(unsigned int flags);
    inline void RemoveConfigFlags(unsigned int flags);
    void AddConversionFlags(unsigned int flags);
    void RemoveConversionFlags(unsigned int flags);

    // ------------- GETTERS ----------------------

    inline std::shared_ptr<Texture> GetTexture() const;
    inline TextureType GetTextureType() const;
    inline unsigned int GetConfigFlags() const;
    inline bool GetConversionFlag(TexConversionFlags flag) const;
    inline unsigned int GetConversionFlags() const;
    inline std::string GetAssetPath() const override;
    inline std::string GetLibraryPath() const override;
    std::string GetLibraryDDSPath() const;

    // ------------- SETTERS ----------------------

    void SetTexture(std::shared_ptr<Texture>& newTexture);
    inline void SetTextureType(TextureType newType);

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

inline std::shared_ptr<Texture> TextureAsset::GetTexture() const
{
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

inline std::string TextureAsset::GetAssetPath() const
{
    return TEXTURES_PATH + GetName();
}

inline std::string TextureAsset::GetLibraryPath() const
{
    return TEXTURES_LIB_PATH + std::to_string(GetUID()) + GENERAL_BINARY_EXTENSION;
}

inline void TextureAsset::SetTextureType(TextureType newType)
{
    _type = newType;
}
