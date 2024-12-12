#pragma once
#include "Asset.h"

class TextureAsset;

enum MatConfig
{
    usePBR = 0x00000001,            // Use PBR or Specular as shaderType
    isOpaque = 0x00000002,          // Is opaque or translucent
};

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
    inline const Color& GetBaseColor() const;
    inline const Color& GetSpecularColor() const;
    inline UINT GetOptions() const;
    inline std::string GetAssetPath() const override;
    inline std::string GetLibraryPath() const override;

    // ------------- SETTERS ----------------------

    inline void SetBaseTexture(std::shared_ptr<TextureAsset>& diffuse);
    inline void SetNormalMap(std::shared_ptr<TextureAsset>& normal);
    inline void SetPropertyTexture(std::shared_ptr<TextureAsset>& metalness);
    inline void SetEmissiveTexture(std::shared_ptr<TextureAsset>& emissive);
    inline void SetAmbientOcclusion(std::shared_ptr<TextureAsset>& occlusion);
    inline void SetBaseColor(Color& color);
    inline void SetSpecularColor(Color& color);
    inline void SetOptions(UINT options);

private:
    std::shared_ptr<TextureAsset> _baseTexture;
    std::shared_ptr<TextureAsset> _normalMap;
    std::shared_ptr<TextureAsset> _propertyTexture;
    std::shared_ptr<TextureAsset> _emissiveTexture;
    std::shared_ptr<TextureAsset> _ambientOcclusion;

    Color _baseColor;
    Color _specularColor;

    UINT _options;
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

inline const Color& MaterialAsset::GetBaseColor() const
{
    return _baseColor;
}

inline const Color& MaterialAsset::GetSpecularColor() const
{
    return _specularColor;
}

inline UINT MaterialAsset::GetOptions() const
{
    return _options;
}

inline std::string MaterialAsset::GetAssetPath() const
{
    return MATERIAL_PATH + GetName();
}

inline std::string MaterialAsset::GetLibraryPath() const
{
    return MATERIAL_LIB_PATH + std::to_string(GetUID()) + GENERAL_BINARY_EXTENSION;
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

inline void MaterialAsset::SetBaseColor(Color& color)
{
    _baseColor = color;
}

inline void MaterialAsset::SetSpecularColor(Color& color)
{
    _baseColor = color;
}

inline void MaterialAsset::SetOptions(UINT options)
{
    _options = options;
}
