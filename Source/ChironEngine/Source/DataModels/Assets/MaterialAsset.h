#pragma once
#include "Asset.h"

class TextureAsset;

class MaterialAsset : public Asset
{
public:
    MaterialAsset();
    ~MaterialAsset() override;

    // ------------- GETTERS ----------------------

    inline TextureAsset* GetDiffuse() const;
    inline TextureAsset* GetNormal() const;
    inline TextureAsset* GetMetalness() const;
    inline TextureAsset* GetEmissive() const;
    inline TextureAsset* GetOcclusion() const;

    // ------------- SETTERS ----------------------

    inline void SetDiffuse(std::shared_ptr<TextureAsset>& diffuse);
    inline void SetNormal(std::shared_ptr<TextureAsset>& normal);
    inline void SetMetalness(std::shared_ptr<TextureAsset>& metalness);
    inline void SetEmissive(std::shared_ptr<TextureAsset>& emissive);
    inline void SetOcclusion(std::shared_ptr<TextureAsset>& occlusion);

private:
    std::shared_ptr<TextureAsset> _textureDiffuse;
    std::shared_ptr<TextureAsset> _textureNormal;
    std::shared_ptr<TextureAsset> _textureMetalness;
    std::shared_ptr<TextureAsset> _textureEmissive;
    std::shared_ptr<TextureAsset> _textureOcclusion;
};

inline TextureAsset* MaterialAsset::GetDiffuse() const
{
    return _textureDiffuse.get();
}

inline TextureAsset* MaterialAsset::GetNormal() const
{
    return _textureNormal.get();
}

inline TextureAsset* MaterialAsset::GetMetalness() const
{
    return _textureMetalness.get();
}

inline TextureAsset* MaterialAsset::GetEmissive() const
{
    return _textureEmissive.get();
}

inline TextureAsset* MaterialAsset::GetOcclusion() const
{
    return _textureOcclusion.get();
}

inline void MaterialAsset::SetDiffuse(std::shared_ptr<TextureAsset>& diffuse)
{
    _textureDiffuse = diffuse;
}

inline void MaterialAsset::SetNormal(std::shared_ptr<TextureAsset>& normal)
{
    _textureNormal = normal;
}

inline void MaterialAsset::SetMetalness(std::shared_ptr<TextureAsset>& metalness)
{
    _textureMetalness = metalness;
}

inline void MaterialAsset::SetEmissive(std::shared_ptr<TextureAsset>& emissive)
{
    _textureEmissive = emissive;
}

inline void MaterialAsset::SetOcclusion(std::shared_ptr<TextureAsset>& occlusion)
{
    _textureOcclusion = occlusion;
}
