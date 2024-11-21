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
    DEPTH,
    RENDER_TARGET
};

class TextureAsset : public Asset
{
public:
    TextureAsset(TextureType type);
    ~TextureAsset() override;

    // ------------- GETTERS ----------------------

    inline std::shared_ptr<Texture> GetTexture() const;
    inline TextureType GetTextureType() const;
    const std::string& GetName() const override;

    // ------------- SETTERS ----------------------

    inline void SetTexture(std::shared_ptr<Texture>& newTexture);
    inline void SetTextureType(TextureType newType);

private:
    std::shared_ptr<Texture> _texture;

    TextureType _type;
};

inline std::shared_ptr<Texture> TextureAsset::GetTexture() const
{
    return _texture;
}

inline TextureType TextureAsset::GetTextureType() const
{
    return _type;
}

inline void TextureAsset::SetTexture(std::shared_ptr<Texture>& newTexture)
{
    _texture = newTexture;
}

inline void TextureAsset::SetTextureType(TextureType newType)
{
    _type = newType;
}
