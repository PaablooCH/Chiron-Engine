#pragma once
#include "Asset.h"

class TextureAsset;

class MaterialAsset : public Asset
{
public:
    MaterialAsset();
    ~MaterialAsset() override;

    // ------------- GETTERS ----------------------

    inline TextureAsset* GetBaseTexture() const;
    inline TextureAsset* GetNormalMap() const;
    inline TextureAsset* GetPropertyTexture() const;
    inline TextureAsset* GetEmissiveTexture() const;
    inline TextureAsset* GetAmbientOcclusion() const;

    // ------------- SETTERS ----------------------

    inline void SetBaseTexture(std::shared_ptr<TextureAsset>& diffuse);
    inline void SetNormalMap(std::shared_ptr<TextureAsset>& normal);
    inline void SetPropertyTexture(std::shared_ptr<TextureAsset>& metalness);
    inline void SetEmissiveTexture(std::shared_ptr<TextureAsset>& emissive);
    inline void SetAmbientOcclusion(std::shared_ptr<TextureAsset>& occlusion);

private:
    std::shared_ptr<TextureAsset> _baseTexture;
    std::shared_ptr<TextureAsset> _normalMap;
    std::shared_ptr<TextureAsset> _propertyTexture;
    std::shared_ptr<TextureAsset> _emissiveTexture;
    std::shared_ptr<TextureAsset> _ambientOcclusion;

};

inline TextureAsset* MaterialAsset::GetBaseTexture() const
{
    return _baseTexture.get();
}

inline TextureAsset* MaterialAsset::GetNormalMap() const
{
    return _normalMap.get();
}

inline TextureAsset* MaterialAsset::GetPropertyTexture() const
{
    return _propertyTexture.get();
}

inline TextureAsset* MaterialAsset::GetEmissiveTexture() const
{
    return _emissiveTexture.get();
}

inline TextureAsset* MaterialAsset::GetAmbientOcclusion() const
{
    return _ambientOcclusion.get();
}
{
    return _textureDiffuse.get();
}

inline TextureAsset* MaterialAsset::GetNormal() const
{
    return _textureNormal.get();
}

inline void MaterialAsset::SetBaseTexture(std::shared_ptr<TextureAsset>& diffuse)
{
    _baseTexture = diffuse;
}

inline void MaterialAsset::SetNormalMap(std::shared_ptr<TextureAsset>& normal)
{
    _normalMap = normal;
}

inline void MaterialAsset::SetPropertyTexture(std::shared_ptr<TextureAsset>& metalness)
{
    _propertyTexture = metalness;
}

inline void MaterialAsset::SetEmissiveTexture(std::shared_ptr<TextureAsset>& emissive)
{
    _emissiveTexture = emissive;
}

inline void MaterialAsset::SetAmbientOcclusion(std::shared_ptr<TextureAsset>& occlusion)
{
    _ambientOcclusion = occlusion;
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
